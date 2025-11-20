from pathlib import Path
from fastapi import APIRouter, Depends, Form, Request, status
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy import select, delete
from sqlalchemy.ext.asyncio import AsyncSession
from datetime import datetime, timedelta

from app.config import settings
from app.database import get_session
from app.models import AccessCode, Lock, PendingDevice
from app.mqtt_client import mqtt_client


templates = Jinja2Templates(directory=str(Path(__file__).resolve().parent / "templates"))
router = APIRouter(prefix="/ui")


def _is_authenticated(request: Request) -> bool:
    return request.session.get("user") == settings.admin_username


def _login_redirect() -> RedirectResponse:
    return RedirectResponse(url="/ui/login", status_code=status.HTTP_303_SEE_OTHER)


def _is_valid_pin(value: str) -> bool:
    return value.isdigit() and 4 <= len(value) <= 10


@router.get("/login", response_class=HTMLResponse)
async def login_page(request: Request):
    if _is_authenticated(request):
        return RedirectResponse(url="/ui/dashboard", status_code=status.HTTP_303_SEE_OTHER)
    return templates.TemplateResponse(
        "login.html",
        {"request": request, "error": request.query_params.get("error")}
    )


@router.post("/login")
async def login(request: Request, username: str = Form(...), password: str = Form(...)):
    if username == settings.admin_username and password == settings.admin_password:
        request.session["user"] = username
        return RedirectResponse(url="/ui/dashboard", status_code=status.HTTP_303_SEE_OTHER)
    return templates.TemplateResponse(
        "login.html",
        {"request": request, "error": "Nieprawidłowe dane logowania"}
    )


@router.get("/logout")
async def logout(request: Request):
    request.session.clear()
    return RedirectResponse(url="/ui/login", status_code=status.HTTP_303_SEE_OTHER)


@router.get("/dashboard", response_class=HTMLResponse)
async def dashboard(
    request: Request,
    session: AsyncSession = Depends(get_session)
):
    if not _is_authenticated(request):
        return _login_redirect()
    result = await session.execute(select(Lock))
    locks = result.scalars().all()
    cutoff = datetime.utcnow() - timedelta(days=1)
    await session.execute(
        delete(PendingDevice).where(PendingDevice.last_seen < cutoff)
    )
    await session.commit()
    pending_result = await session.execute(
        select(PendingDevice)
        .where(PendingDevice.last_seen >= cutoff)
        .order_by(PendingDevice.last_seen.desc())
    )
    pending_devices = pending_result.scalars().all()
    return templates.TemplateResponse(
        "dashboard.html",
        {
            "request": request,
            "locks": locks,
            "pending_devices": pending_devices,
            "message": request.query_params.get("message"),
            "error": request.query_params.get("error")
        }
    )


@router.get("/locks/new", response_class=HTMLResponse)
async def new_lock(request: Request):
    if not _is_authenticated(request):
        return _login_redirect()
    device_prefill = request.query_params.get("device_id", "")
    return templates.TemplateResponse(
        "lock_new.html",
        {
            "request": request,
            "error": request.query_params.get("error"),
            "device_prefill": device_prefill
        }
    )


@router.post("/locks")
async def create_lock_ui(
    request: Request,
    session: AsyncSession = Depends(get_session),
    device_id: str = Form(...),
    name: str = Form(...),
    location: str = Form(None)
):
    if not _is_authenticated(request):
        return _login_redirect()
    device_id = device_id.strip()
    clean_name = name.strip()
    clean_location = location.strip() if location else None
    if not device_id or not clean_name:
        return RedirectResponse(
            url="/ui/locks/new?error=missing_fields",
            status_code=status.HTTP_303_SEE_OTHER
        )
    result = await session.execute(select(Lock).where(Lock.device_id == device_id))
    if result.scalar_one_or_none():
        return RedirectResponse(
            url="/ui/locks/new?error=duplicate",
            status_code=status.HTTP_303_SEE_OTHER
        )
    lock = Lock(
        device_id=device_id,
        name=clean_name,
        location=clean_location
    )
    session.add(lock)
    await session.commit()
    await session.refresh(lock)
    await session.execute(
        delete(PendingDevice).where(PendingDevice.device_id == device_id)
    )
    await session.commit()
    return RedirectResponse(
        url=f"/ui/dashboard?message=lock_created",
        status_code=status.HTTP_303_SEE_OTHER
    )


@router.get("/locks/{lock_id}", response_class=HTMLResponse)
async def lock_detail(
    lock_id: int,
    request: Request,
    session: AsyncSession = Depends(get_session)
):
    if not _is_authenticated(request):
        return _login_redirect()
    lock_result = await session.execute(select(Lock).where(Lock.id == lock_id))
    lock = lock_result.scalar_one_or_none()
    if not lock:
        return RedirectResponse(url="/ui/dashboard?error=not_found", status_code=status.HTTP_303_SEE_OTHER)
    codes_result = await session.execute(
        select(AccessCode).where(AccessCode.lock_id == lock_id).order_by(AccessCode.created_at.desc())
    )
    codes = codes_result.scalars().all()
    return templates.TemplateResponse(
        "lock_detail.html",
        {
            "request": request,
            "lock": lock,
            "codes": codes,
            "message": request.query_params.get("message"),
            "error": request.query_params.get("error")
        }
    )


@router.post("/locks/{lock_id}/access-codes")
async def create_access_code_ui(
    lock_id: int,
    request: Request,
    session: AsyncSession = Depends(get_session),
    code: str = Form(...),
    name: str = Form(None)
):
    if not _is_authenticated(request):
        return _login_redirect()
    code = code.strip()
    clean_name = name.strip() if name else None
    if not _is_valid_pin(code):
        return RedirectResponse(
            url=f"/ui/locks/{lock_id}?error=invalid_pin",
            status_code=status.HTTP_303_SEE_OTHER
        )
    lock_result = await session.execute(select(Lock).where(Lock.id == lock_id))
    lock = lock_result.scalar_one_or_none()
    if not lock:
        return RedirectResponse(url="/ui/dashboard?error=not_found", status_code=status.HTTP_303_SEE_OTHER)
    new_code = AccessCode(lock_id=lock_id, code=code, name=clean_name)
    session.add(new_code)
    await session.commit()
    await session.refresh(new_code)
    mqtt_client.request_sync(lock.device_id)
    return RedirectResponse(
        url=f"/ui/locks/{lock_id}?message=code_created",
        status_code=status.HTTP_303_SEE_OTHER
    )


@router.post("/locks/{lock_id}/quick-pin")
async def quick_pin_update_ui(
    lock_id: int,
    request: Request,
    session: AsyncSession = Depends(get_session),
    code: str = Form(...),
    name: str = Form(None)
):
    if not _is_authenticated(request):
        return _login_redirect()
    code = code.strip()
    clean_name = name.strip() if name else None
    if not _is_valid_pin(code):
        return RedirectResponse(
            url="/ui/dashboard?error=invalid_pin",
            status_code=status.HTTP_303_SEE_OTHER
        )
    lock_result = await session.execute(select(Lock).where(Lock.id == lock_id))
    lock = lock_result.scalar_one_or_none()
    if not lock:
        return RedirectResponse(url="/ui/dashboard?error=not_found", status_code=status.HTTP_303_SEE_OTHER)
    label = clean_name or "PIN główny"
    existing_result = await session.execute(
        select(AccessCode).where(
            AccessCode.lock_id == lock_id,
            AccessCode.name == label
        ).limit(1)
    )
    access_code = existing_result.scalar_one_or_none()
    if access_code:
        access_code.code = code
        access_code.name = label
        access_code.is_active = True
        message = "pin_updated"
    else:
        access_code = AccessCode(lock_id=lock_id, code=code, name=label)
        session.add(access_code)
        message = "pin_created"
    await session.commit()
    mqtt_client.request_sync(lock.device_id)
    return RedirectResponse(
        url=f"/ui/dashboard?message={message}",
        status_code=status.HTTP_303_SEE_OTHER
    )


@router.post("/access-codes/{code_id}/update")
async def update_access_code_ui(
    code_id: int,
    request: Request,
    session: AsyncSession = Depends(get_session),
    code: str = Form(...),
    name: str = Form(None),
    is_active: str = Form("true")
):
    if not _is_authenticated(request):
        return _login_redirect()
    code = code.strip()
    clean_name = name.strip() if name else None
    code_result = await session.execute(select(AccessCode).where(AccessCode.id == code_id))
    access_code = code_result.scalar_one_or_none()
    if not access_code:
        return RedirectResponse(url="/ui/dashboard?error=not_found", status_code=status.HTTP_303_SEE_OTHER)
    if not _is_valid_pin(code):
        return RedirectResponse(
            url=f"/ui/locks/{access_code.lock_id}?error=invalid_pin",
            status_code=status.HTTP_303_SEE_OTHER
        )
    access_code.code = code
    access_code.name = clean_name
    access_code.is_active = is_active.lower() == "true"
    await session.commit()
    await session.refresh(access_code)
    lock_result = await session.execute(select(Lock).where(Lock.id == access_code.lock_id))
    lock = lock_result.scalar_one_or_none()
    if lock:
        mqtt_client.request_sync(lock.device_id)
    return RedirectResponse(
        url=f"/ui/locks/{access_code.lock_id}?message=code_updated",
        status_code=status.HTTP_303_SEE_OTHER
    )
