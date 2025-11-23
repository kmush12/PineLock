from pathlib import Path
from fastapi import APIRouter, Depends, Form, Request, status
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy import select, delete
from sqlalchemy.ext.asyncio import AsyncSession
from datetime import datetime, timedelta
import logging

from app.config import settings
from app.database import get_session
from app.models import AccessCode, Lock, PendingDevice
from app.mqtt_client import mqtt_client

logger = logging.getLogger(__name__)


templates = Jinja2Templates(directory=str(Path(__file__).resolve().parent / "templates"))
router = APIRouter(prefix="/ui")


def _is_authenticated(request: Request) -> bool:
    return request.session.get("user") == settings.admin_username


def _login_redirect() -> RedirectResponse:
    return RedirectResponse(url="/ui/login", status_code=status.HTTP_303_SEE_OTHER)


def _is_valid_pin(value: str) -> bool:
    return value.isdigit() and 4 <= len(value) <= 10


def _get_user_context(request: Request) -> dict:
    username = request.session.get("user", "Admin")
    user_initial = username[0].upper() if username else "A"
    return {"username": username, "user_initial": user_initial}


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
async def dashboard(request: Request):
    if not _is_authenticated(request):
        return _login_redirect()
    return RedirectResponse(url="/ui/locks", status_code=status.HTTP_303_SEE_OTHER)


@router.get("/locks", response_class=HTMLResponse)
async def locks_list(request: Request, session: AsyncSession = Depends(get_session)):
    if not _is_authenticated(request):
        return _login_redirect()
    
    # Get username for display
    username = request.session.get("user", "Admin")
    user_initial = username[0].upper() if username else "A"
    
    # Get all locks
    result = await session.execute(select(Lock))
    locks = result.scalars().all()
    locks_json = []
    for lock in locks:
        lock_dict = {}
        for k, v in lock.__dict__.items():
            if not k.startswith('_'):
                if isinstance(v, datetime):
                    lock_dict[k] = v.strftime("%d.%m.%Y %H:%M")
                else:
                    lock_dict[k] = v
        locks_json.append(lock_dict)

    # Calculate stats
    total_locks = len(locks)
    locked_count = sum(1 for lock in locks if lock.is_locked)
    unlocked_count = sum(1 for lock in locks if not lock.is_locked and lock.is_online)
    offline_count = sum(1 for lock in locks if not lock.is_online)

    # Check for messages
    message = None
    if request.query_params.get("deleted") == "1":
        message = "✅ Domek został usunięty"

    recent_logs = []
    
    return templates.TemplateResponse(
        "dashboard.html",
        {
            "request": request,
            "username": username,
            "user_initial": user_initial,
            "locks": locks_json,
            "total_locks": total_locks,
            "locked_count": locked_count,
            "unlocked_count": unlocked_count,
            "offline_count": offline_count,
            "message": message,
            "recent_logs": recent_logs,
            "active_page": "domki",
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
            **_get_user_context(request),
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
    location: str = Form(None),
    description: str = Form(None)
):
    if not _is_authenticated(request):
        return _login_redirect()
    device_id = device_id.strip()
    clean_name = name.strip()
    clean_location = location.strip() if location else None
    clean_description = description.strip() if description else None
    
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
        location=clean_location,
        description=clean_description
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
    
    # Prepare access methods display (PIN codes + RFID cards)
    access_methods = []
    for code in codes:
        if code.code is not None:  # PIN
            access_methods.append({
                'id': code.id,
                'type': 'pin',
                'identifier': code.code,
                'description': code.name or 'Bez nazwy',
                'valid_from': code.created_at.strftime('%Y-%m-%d %H:%M') if code.created_at else '-',
                'valid_to': None,
                'is_active': code.is_active
            })
        elif code.card_uid is not None:  # RFID
            access_methods.append({
                'id': code.id,
                'type': 'rfid',
                'identifier': code.card_uid,
                'description': code.name or 'Bez nazwy',
                'valid_from': code.created_at.strftime('%Y-%m-%d %H:%M') if code.created_at else '-',
                'valid_to': None,
                'is_active': code.is_active
            })
    
    # Check if PIN and RFID exist
    has_pin = any(m['type'] == 'pin' for m in access_methods)
    has_rfid = any(m['type'] == 'rfid' for m in access_methods)

    # Access history (placeholder)
    access_history = []

    return templates.TemplateResponse(
        "lock_detail.html",
        {
            "request": request,
            **_get_user_context(request),
            "lock": lock,
            "codes": codes,
            "access_methods": access_methods,
            "has_pin": has_pin,
            "has_rfid": has_rfid,
            "access_methods": access_methods,
            "access_history": access_history,
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


@router.get("/access", response_class=HTMLResponse)
async def access_management(request: Request, session: AsyncSession = Depends(get_session)):
    if not _is_authenticated(request):
        return _login_redirect()
    
    from app.models import RFIDCard
    
    # Fetch Master PIN (only 1 should exist)
    master_pin_result = await session.execute(select(AccessCode).where(AccessCode.lock_id.is_(None)))
    master_pin = master_pin_result.scalar_one_or_none()
    
    # Fetch Locks with their PIN and Key Tag
    locks_result = await session.execute(select(Lock).order_by(Lock.name))
    locks = locks_result.scalars().all()
    
    # Fetch all non-master PINs
    pins_result = await session.execute(select(AccessCode).where(AccessCode.lock_id.is_not(None)))
    all_pins = pins_result.scalars().all()
    
    # Fetch all Key Tags
    key_tags_result = await session.execute(select(RFIDCard).where(RFIDCard.card_type == 'key_tag'))
    all_key_tags = key_tags_result.scalars().all()
    
    # Group by lock (single PIN + single Key Tag per lock)
    locks_data = []
    for lock in locks:
        lock_pin = next((p for p in all_pins if p.lock_id == lock.id), None)
        key_tag = next((r for r in all_key_tags if r.lock_id == lock.id), None)
        
        locks_data.append({
            "lock": lock,
            "pin": lock_pin,  # Single PIN or None
            "key_tag": key_tag  # Single Key Tag or None
        })
        
    return templates.TemplateResponse(
        "access_management.html",
        {
            "request": request,
            **_get_user_context(request),
            "active_page": "access",
            "master_pin": master_pin,  # Single or None
            "locks_data": locks_data,
            "message": request.query_params.get("message"),
            "error": request.query_params.get("error")
        }
    )

# Redirect old routes
@router.get("/access-codes")
async def access_codes_redirect():
    return RedirectResponse(url="/ui/access", status_code=status.HTTP_301_MOVED_PERMANENTLY)

@router.get("/rfid-cards")
async def rfid_cards_redirect():
    return RedirectResponse(url="/ui/access", status_code=status.HTTP_301_MOVED_PERMANENTLY)


@router.get("/access-logs", response_class=HTMLResponse)
async def access_logs_list(request: Request, session: AsyncSession = Depends(get_session)):
    if not _is_authenticated(request):
        return _login_redirect()
    
    from app.models import AccessLog, RFIDCard
    
    # Get filter parameters
    filter_lock_id = request.query_params.get("lock_id")
    filter_access_type = request.query_params.get("access_type")
    filter_success = request.query_params.get("success")
    
    # Base query
    query = select(AccessLog).join(Lock)
    
    # Apply filters
    if filter_lock_id:
        query = query.where(AccessLog.lock_id == int(filter_lock_id))
    if filter_access_type:
        query = query.where(AccessLog.access_type == filter_access_type)
    if filter_success == "true":
        query = query.where(AccessLog.success == True)
    elif filter_success == "false":
        query = query.where(AccessLog.success == False)
    
    query = query.order_by(AccessLog.timestamp.desc()).limit(100)
    
    logs_result = await session.execute(query)
    logs = logs_result.scalars().all()
    
    # Get all locks for filter dropdown
    locks_result = await session.execute(select(Lock).order_by(Lock.name))
    all_locks = locks_result.scalars().all()
    
    # Calculate stats
    total_logs = len(logs)
    successful_attempts = sum(1 for log in logs if log.success)
    failed_attempts = total_logs - successful_attempts
    
    # Count today's logs
    from datetime import datetime, timedelta
    today = datetime.utcnow().date()
    today_count = sum(1 for log in logs if log.timestamp and log.timestamp.date() == today)
    
    return templates.TemplateResponse(
        "access_logs.html",
        {
            "request": request,
            **_get_user_context(request),
            "logs": logs,
            "all_locks": all_locks,
            "total_logs": total_logs,
            "successful_attempts": successful_attempts,
            "failed_attempts": failed_attempts,
            "today_count": today_count,
            "filter_lock_id": int(filter_lock_id) if filter_lock_id else None,
            "filter_access_type": filter_access_type,
            "filter_success": True if filter_success == "true" else (False if filter_success == "false" else None),
            "has_more": False,
            "active_page": "access-logs",
            "message": request.query_params.get("message"),
            "error": request.query_params.get("error")
        }
    )


@router.get("/settings", response_class=HTMLResponse)
async def settings_page(request: Request, session: AsyncSession = Depends(get_session)):
    if not _is_authenticated(request):
        return _login_redirect()
    
    from app.models import RFIDCard, AccessLog
    
    username = request.session.get("user", "Admin")
    
    # System info
    system_info = {
        "version": settings.api_version,
        "api_host": settings.api_host,
        "api_port": settings.api_port,
        "database_url": settings.database_url,
        "session_secret_key": settings.session_secret_key
    }
    
    # MQTT config
    mqtt_config = {
        "host": settings.mqtt_broker_host,
        "port": settings.mqtt_broker_port,
        "username": settings.mqtt_username,
        "topic_prefix": settings.mqtt_topic_prefix
    }
    
    # Database stats
    locks_count_result = await session.execute(select(Lock))
    locks_count = len(locks_count_result.scalars().all())
    
    codes_count_result = await session.execute(select(AccessCode))
    codes_count = len(codes_count_result.scalars().all())
    
    rfid_count_result = await session.execute(select(RFIDCard))
    rfid_count = len(rfid_count_result.scalars().all())
    
    logs_count_result = await session.execute(select(AccessLog))
    logs_count = len(logs_count_result.scalars().all())
    
    db_stats = {
        "locks_count": locks_count,
        "codes_count": codes_count,
        "rfid_count": rfid_count,
        "logs_count": logs_count
    }
    
    return templates.TemplateResponse(
        "settings.html",
        {
            "request": request,
            **_get_user_context(request),
            "system_info": system_info,
            "mqtt_config": mqtt_config,
            "db_stats": db_stats,
            "active_page": "settings",
            "message": request.query_params.get("message"),
            "error": request.query_params.get("error")
        }
    )


@router.post("/settings/password")
async def change_password(
    request: Request,
    current_password: str = Form(...),
    new_password: str = Form(...),
    confirm_password: str = Form(...)
):
    if not _is_authenticated(request):
        return _login_redirect()
    
    # Verify current password
    if current_password != settings.admin_password:
        return RedirectResponse(
            url="/ui/settings?error=invalid_credentials",
            status_code=status.HTTP_303_SEE_OTHER
        )
    
    # Verify new passwords match
    if new_password != confirm_password:
        return RedirectResponse(
            url="/ui/settings?error=password_mismatch",
            status_code=status.HTTP_303_SEE_OTHER
        )
    
    # TODO: Save new password to .env file
    # For now, just show a message that it would need to be changed in .env
    return RedirectResponse(
        url="/ui/settings?error=password_change_requires_env_update",
        status_code=status.HTTP_303_SEE_OTHER
    )



