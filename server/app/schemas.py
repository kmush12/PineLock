from pydantic import BaseModel, Field
from typing import Optional
from datetime import datetime


# Lock Schemas
class LockBase(BaseModel):
    device_id: str
    name: str
    location: Optional[str] = None
    description: Optional[str] = None


class LockCreate(LockBase):
    pass


class LockUpdate(BaseModel):
    name: Optional[str] = None
    location: Optional[str] = None
    description: Optional[str] = None


class LockResponse(LockBase):
    id: int
    is_online: bool
    is_locked: bool
    is_key_present: bool
    last_seen: datetime
    created_at: datetime

    class Config:
        from_attributes = True


# Access Code Schemas
class AccessCodeBase(BaseModel):
    code: str = Field(..., min_length=4, max_length=10)
    name: Optional[str] = None
    is_active: bool = True
    valid_from: Optional[datetime] = None
    valid_until: Optional[datetime] = None


class AccessCodeCreate(AccessCodeBase):
    lock_id: Optional[int] = None


class AccessCodeUpdate(BaseModel):
    code: Optional[str] = Field(None, min_length=4, max_length=10)
    name: Optional[str] = None
    is_active: Optional[bool] = None
    valid_from: Optional[datetime] = None
    valid_until: Optional[datetime] = None


class AccessCodeResponse(AccessCodeBase):
    id: int
    lock_id: Optional[int]
    created_at: datetime
    
    class Config:
        from_attributes = True


# RFID Card Schemas
class RFIDCardBase(BaseModel):
    card_uid: str
    name: Optional[str] = None
    card_type: str = "key_tag"  # Only 'key_tag' allowed (for presence detection)
    is_active: bool = True
    valid_from: Optional[datetime] = None
    valid_until: Optional[datetime] = None


class RFIDCardCreate(RFIDCardBase):
    lock_id: Optional[int] = None  # Required for key_tag


class RFIDCardUpdate(BaseModel):
    name: Optional[str] = None
    card_uid: Optional[str] = None
    is_active: Optional[bool] = None
    valid_from: Optional[datetime] = None
    valid_until: Optional[datetime] = None


class RFIDCardResponse(RFIDCardBase):
    id: int
    lock_id: Optional[int]
    created_at: datetime
    
    class Config:
        from_attributes = True


# Access Log Schemas
class AccessLogResponse(BaseModel):
    id: int
    lock_id: int
    access_type: str
    access_method: Optional[str]
    success: bool
    timestamp: datetime
    
    class Config:
        from_attributes = True


# Lock Command Schemas
class LockCommand(BaseModel):
    action: str = Field(..., pattern="^(lock|unlock)$")


# MQTT Message Schemas
class MQTTAccessEvent(BaseModel):
    device_id: str
    access_type: str  # 'pin', 'rfid', 'remote'
    access_method: Optional[str]  # PIN code or RFID UID
    success: bool
    timestamp: Optional[datetime] = None


class MQTTStatusUpdate(BaseModel):
    device_id: str
    is_locked: bool
    is_key_present: Optional[bool] = None  # RFID key presence status
    timestamp: Optional[datetime] = None
