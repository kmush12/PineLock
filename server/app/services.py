import logging
from sqlalchemy import select, or_
from app.database import async_session_maker
from app.models import Lock, AccessCode, RFIDCard
from app.mqtt_client import mqtt_client

logger = logging.getLogger(__name__)

async def sync_device(device_id: str):
    """
    Gather configuration for a device and push it via MQTT.
    Includes:
    - Access Codes (PINs): lock-specific + master PIN
    - RFID Cards: lock-specific only
    - Key Tag: if assigned to this lock
    """
    try:
        async with async_session_maker() as session:
            # 1. Get Lock
            result = await session.execute(select(Lock).where(Lock.device_id == device_id))
            lock = result.scalar_one_or_none()
            
            if not lock:
                logger.error(f"Cannot sync unknown device: {device_id}")
                return

            # 2. Get Access Codes (lock-specific + master PIN)
            result = await session.execute(
                select(AccessCode).where(
                    AccessCode.is_active == True,
                    or_(
                        AccessCode.lock_id == lock.id,
                        AccessCode.lock_id == None  # Master PIN
                    )
                )
            )
            access_codes = [code.code for code in result.scalars().all()]

            # 3. Get RFID Cards
            # We need:
            # - Cards assigned to this lock (lock_id == lock.id)
            # - Key tags are handled separately to identify which UID is the Key Tag
            
            # Fetch all active cards relevant to this lock
            result = await session.execute(
                select(RFIDCard).where(
                    RFIDCard.is_active == True,
                    RFIDCard.lock_id == lock.id
                )
            )
            all_cards = result.scalars().all()
            
            rfid_access_list = []
            key_tag_uid = None
            
            for card in all_cards:
                if card.card_type == 'key_tag':
                    # This is the key tag for THIS lock
                    key_tag_uid = card.card_uid
                else:
                    # Regular access card for this lock
                    rfid_access_list.append(card.card_uid)

            # 4. Construct Payload
            payload = {
                "access_codes": access_codes,
                "rfid_cards": rfid_access_list,
                "key_tag": key_tag_uid
            }
            
            # 5. Publish Config
            logger.info(f"Syncing config to {device_id}: {len(access_codes)} PINs, {len(rfid_access_list)} Cards, KeyTag: {key_tag_uid}")
            mqtt_client.publish(device_id, "config", payload)

    except Exception as e:
        logger.error(f"Error syncing device {device_id}: {e}")
