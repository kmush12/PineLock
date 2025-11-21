from fastapi import APIRouter, Depends, HTTPException, status
from pathlib import Path
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from typing import List
from datetime import datetime

from app.database import get_session
from app.models import Lock, AccessCode, RFIDCard, AccessLog
import logging
from datetime import datetime
from app.schemas import (
    LockCreate, LockUpdate, LockResponse,
    AccessCodeCreate, AccessCodeUpdate, AccessCodeResponse,
    RFIDCardCreate, RFIDCardUpdate, RFIDCardResponse,
    AccessLogResponse, LockCommand
)
from app.mqtt_client import mqtt_client

router = APIRouter()


# Lock Endpoints
@router.get("/locks", response_model=List[LockResponse])
async def list_locks(session: AsyncSession = Depends(get_session)):
    """Get list of all locks with their current status."""
    result = await session.execute(select(Lock))
    locks = result.scalars().all()
    return locks


@router.post("/locks", response_model=LockResponse, status_code=status.HTTP_201_CREATED)
async def create_lock(lock: LockCreate, session: AsyncSession = Depends(get_session)):
    """Create a new lock."""
    # Check if device_id already exists
    result = await session.execute(
        select(Lock).where(Lock.device_id == lock.device_id)
    )
    if result.scalar_one_or_none():
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Lock with this device_id already exists"
        )
    
    db_lock = Lock(**lock.dict())
    session.add(db_lock)
    await session.commit()
    await session.refresh(db_lock)
    return db_lock


@router.get("/locks/{lock_id}", response_model=LockResponse)
async def get_lock(lock_id: int, session: AsyncSession = Depends(get_session)):
    """Get a specific lock by ID."""
    result = await session.execute(select(Lock).where(Lock.id == lock_id))
    lock = result.scalar_one_or_none()
    if not lock:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Lock not found")
    return lock


@router.put("/locks/{lock_id}", response_model=LockResponse)
async def update_lock(
    lock_id: int,
    lock_update: LockUpdate,
    session: AsyncSession = Depends(get_session)
):
    """Update a lock."""
    result = await session.execute(select(Lock).where(Lock.id == lock_id))
    lock = result.scalar_one_or_none()
    if not lock:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Lock not found")
    
    update_data = lock_update.dict(exclude_unset=True)
    for field, value in update_data.items():
        setattr(lock, field, value)
    
    await session.commit()
    await session.refresh(lock)
    return lock


@router.delete("/locks/{lock_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_lock(lock_id: int, session: AsyncSession = Depends(get_session)):
    """Delete a lock."""
    result = await session.execute(select(Lock).where(Lock.id == lock_id))
    lock = result.scalar_one_or_none()
    if not lock:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Lock not found")
    
    await session.delete(lock)
    await session.commit()
    return None


@router.post("/locks/{lock_id}/command")
async def send_lock_command(
    lock_id: int,
    command: LockCommand,
    session: AsyncSession = Depends(get_session)
):
    """Send lock/unlock command to a device."""
    result = await session.execute(select(Lock).where(Lock.id == lock_id))
    lock = result.scalar_one_or_none()
    if not lock:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Lock not found")
    
    # Send command via MQTT
    success = mqtt_client.send_lock_command(lock.device_id, command.action)
    if not success:
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Failed to send command to device"
        )
    
    return {"status": "command_sent", "action": command.action}


# Access Code Endpoints
@router.get("/locks/{lock_id}/access-codes", response_model=List[AccessCodeResponse])
async def list_access_codes(lock_id: int, session: AsyncSession = Depends(get_session)):
    """Get all access codes for a lock."""
    result = await session.execute(
        select(AccessCode).where(AccessCode.lock_id == lock_id)
    )
    codes = result.scalars().all()
    return codes


@router.post("/access-codes", response_model=AccessCodeResponse, status_code=status.HTTP_201_CREATED)
async def create_access_code(
    access_code: AccessCodeCreate,
    session: AsyncSession = Depends(get_session)
):
    """Create a new access code."""
    # Verify lock exists
    result = await session.execute(select(Lock).where(Lock.id == access_code.lock_id))
    lock = result.scalar_one_or_none()
    if not lock:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Lock not found")
    
    db_code = AccessCode(**access_code.dict())
    session.add(db_code)
    await session.commit()
    await session.refresh(db_code)
    
    # Request device to sync
    mqtt_client.request_sync(lock.device_id)
    
    return db_code


@router.put("/access-codes/{code_id}", response_model=AccessCodeResponse)
async def update_access_code(
    code_id: int,
    code_update: AccessCodeUpdate,
    session: AsyncSession = Depends(get_session)
):
    """Update an access code."""
    result = await session.execute(select(AccessCode).where(AccessCode.id == code_id))
    code = result.scalar_one_or_none()
    if not code:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Access code not found")
    
    update_data = code_update.dict(exclude_unset=True)
    for field, value in update_data.items():
        setattr(code, field, value)
    
    await session.commit()
    await session.refresh(code)
    
    # Get lock and request sync
    lock_result = await session.execute(select(Lock).where(Lock.id == code.lock_id))
    lock = lock_result.scalar_one_or_none()
    if lock:
        mqtt_client.request_sync(lock.device_id)
    
    return code


@router.delete("/access-codes/{code_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_access_code(code_id: int, session: AsyncSession = Depends(get_session)):
    """Delete an access code."""
    result = await session.execute(select(AccessCode).where(AccessCode.id == code_id))
    code = result.scalar_one_or_none()
    if not code:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Access code not found")
    
    lock_id = code.lock_id
    await session.delete(code)
    await session.commit()
    
    # Get lock and request sync
    lock_result = await session.execute(select(Lock).where(Lock.id == lock_id))
    lock = lock_result.scalar_one_or_none()
    if lock:
        mqtt_client.request_sync(lock.device_id)
    
    return None


# RFID Card Endpoints
@router.get("/locks/{lock_id}/rfid-cards", response_model=List[RFIDCardResponse])
async def list_rfid_cards(lock_id: int, session: AsyncSession = Depends(get_session)):
    """Get all RFID cards for a lock."""
    result = await session.execute(
        select(RFIDCard).where(RFIDCard.lock_id == lock_id)
    )
    cards = result.scalars().all()
    return cards


@router.post("/rfid-cards", response_model=RFIDCardResponse, status_code=status.HTTP_201_CREATED)
async def create_rfid_card(
    rfid_card: RFIDCardCreate,
    session: AsyncSession = Depends(get_session)
):
    """Create a new RFID card."""
    # Verify lock exists
    result = await session.execute(select(Lock).where(Lock.id == rfid_card.lock_id))
    lock = result.scalar_one_or_none()
    if not lock:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Lock not found")
    
    db_card = RFIDCard(**rfid_card.dict())
    session.add(db_card)
    await session.commit()
    await session.refresh(db_card)
    
    # Request device to sync
    mqtt_client.request_sync(lock.device_id)
    
    return db_card


@router.put("/rfid-cards/{card_id}", response_model=RFIDCardResponse)
async def update_rfid_card(
    card_id: int,
    card_update: RFIDCardUpdate,
    session: AsyncSession = Depends(get_session)
):
    """Update an RFID card."""
    result = await session.execute(select(RFIDCard).where(RFIDCard.id == card_id))
    card = result.scalar_one_or_none()
    if not card:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="RFID card not found")
    
    update_data = card_update.dict(exclude_unset=True)
    for field, value in update_data.items():
        setattr(card, field, value)
    
    await session.commit()
    await session.refresh(card)
    
    # Get lock and request sync
    lock_result = await session.execute(select(Lock).where(Lock.id == card.lock_id))
    lock = lock_result.scalar_one_or_none()
    if lock:
        mqtt_client.request_sync(lock.device_id)
    
    return card


@router.delete("/rfid-cards/{card_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_rfid_card(card_id: int, session: AsyncSession = Depends(get_session)):
    """Delete an RFID card."""
    result = await session.execute(select(RFIDCard).where(RFIDCard.id == card_id))
    card = result.scalar_one_or_none()
    if not card:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="RFID card not found")
    
    lock_id = card.lock_id
    await session.delete(card)
    await session.commit()
    
    # Get lock and request sync
    lock_result = await session.execute(select(Lock).where(Lock.id == lock_id))
    lock = lock_result.scalar_one_or_none()
    if lock:
        mqtt_client.request_sync(lock.device_id)
    
    return None


# Access Log Endpoints
@router.get("/locks/{lock_id}/access-logs", response_model=List[AccessLogResponse])
async def list_access_logs(
    lock_id: int,
    limit: int = 100,
    session: AsyncSession = Depends(get_session)
):
    """Get access logs for a lock."""
    result = await session.execute(
        select(AccessLog)
        .where(AccessLog.lock_id == lock_id)
        .order_by(AccessLog.timestamp.desc())
        .limit(limit)
    )
    logs = result.scalars().all()
    return logs


# Log Endpoints
@router.get("/logs/server")
async def get_server_logs(limit: int = 100):
    """Get server logs."""
    logs = []
    log_file = Path("server.log")
    
    if log_file.exists():
        try:
            with open(log_file, "r") as f:
                lines = f.readlines()
                # Process last N lines
                for line in lines[-limit:]:
                    try:
                        # Parse line: 2024-05-21 10:00:00,000 - app.main - INFO - Message
                        parts = line.split(" - ")
                        if len(parts) >= 4:
                            timestamp = parts[0]
                            module = parts[1]
                            level = parts[2]
                            message = " - ".join(parts[3:]).strip()
                            logs.append({
                                "timestamp": timestamp,
                                "level": level,
                                "module": module,
                                "message": message
                            })
                    except Exception:
                        continue
        except Exception as e:
            logging.error(f"Error reading log file: {e}")
            
    return list(reversed(logs))


@router.get("/logs/nodes")
async def get_node_logs(limit: int = 100):
    """Get logs from all connected nodes."""
    # For now, return empty list - can be extended to collect logs via MQTT
    return []
