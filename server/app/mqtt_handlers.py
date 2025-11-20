import logging
from datetime import datetime
from sqlalchemy import select
from app.database import async_session_maker
from app.models import Lock, AccessLog, PendingDevice
from app.schemas import MQTTAccessEvent, MQTTStatusUpdate

logger = logging.getLogger(__name__)


async def handle_status_update(device_id: str, data: dict):
    """Handle status update from device."""
    try:
        status = MQTTStatusUpdate(device_id=device_id, **data)
        
        async with async_session_maker() as session:
            # Find lock by device_id
            result = await session.execute(
                select(Lock).where(Lock.device_id == device_id)
            )
            lock = result.scalar_one_or_none()
            
            if lock:
                lock.is_locked = status.is_locked
                if status.is_key_present is not None:
                    lock.is_key_present = status.is_key_present
                lock.is_online = True
                lock.last_seen = datetime.utcnow()
                await session.commit()
                logger.info(f"Updated status for lock {device_id}: locked={status.is_locked}, key_present={status.is_key_present}")
            else:
                logger.warning(f"Received status from unknown device: {device_id}")
                await _track_pending_device(session, device_id)
    
    except Exception as e:
        logger.error(f"Error handling status update: {e}")


async def handle_access_event(device_id: str, data: dict):
    """Handle access event from device."""
    try:
        event = MQTTAccessEvent(device_id=device_id, **data)
        
        async with async_session_maker() as session:
            # Find lock by device_id
            result = await session.execute(
                select(Lock).where(Lock.device_id == device_id)
            )
            lock = result.scalar_one_or_none()
            
            if lock:
                # Create access log
                log = AccessLog(
                    lock_id=lock.id,
                    access_type=event.access_type,
                    access_method=event.access_method,
                    success=event.success,
                    timestamp=event.timestamp or datetime.utcnow()
                )
                session.add(log)
                
                # Update last seen
                lock.last_seen = datetime.utcnow()
                lock.is_online = True
                
                await session.commit()
                
                logger.info(
                    f"Logged access event for lock {device_id}: "
                    f"type={event.access_type}, success={event.success}"
                )
            else:
                logger.warning(f"Received access event from unknown device: {device_id}")
                await _track_pending_device(session, device_id)
    
    except Exception as e:
        logger.error(f"Error handling access event: {e}")


async def handle_heartbeat(device_id: str, data: dict):
    """Handle heartbeat from device."""
    try:
        async with async_session_maker() as session:
            # Find lock by device_id
            result = await session.execute(
                select(Lock).where(Lock.device_id == device_id)
            )
            lock = result.scalar_one_or_none()
            
            if lock:
                lock.is_online = True
                lock.last_seen = datetime.utcnow()
                await session.commit()
                logger.debug(f"Received heartbeat from lock {device_id}")
            else:
                logger.warning(f"Received heartbeat from unknown device: {device_id}")
                await _track_pending_device(session, device_id)
    
    except Exception as e:
        logger.error(f"Error handling heartbeat: {e}")


def setup_mqtt_handlers(mqtt_client):
    """Register MQTT message handlers."""
    mqtt_client.register_handler("status", handle_status_update)
    mqtt_client.register_handler("access", handle_access_event)
    mqtt_client.register_handler("heartbeat", handle_heartbeat)
async def _track_pending_device(session, device_id: str):
    """Record or update pending domek entries."""
    clean_device_id = device_id.strip()
    result = await session.execute(
        select(PendingDevice).where(PendingDevice.device_id == clean_device_id)
    )
    pending = result.scalar_one_or_none()
    now = datetime.utcnow()
    if pending:
        pending.last_seen = now
    else:
        pending = PendingDevice(device_id=clean_device_id, first_seen=now, last_seen=now)
        session.add(pending)
    await session.commit()
