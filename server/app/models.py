from sqlalchemy import Column, Integer, String, Boolean, DateTime, ForeignKey
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship
from datetime import datetime

Base = declarative_base()


class Lock(Base):
    """Lock device model."""
    __tablename__ = "locks"
    
    id = Column(Integer, primary_key=True, index=True)
    device_id = Column(String, unique=True, index=True, nullable=False)
    name = Column(String, nullable=False)
    location = Column(String)
    is_online = Column(Boolean, default=False)
    is_locked = Column(Boolean, default=True)
    last_seen = Column(DateTime, default=datetime.utcnow)
    created_at = Column(DateTime, default=datetime.utcnow)
    
    # Relationships
    access_codes = relationship("AccessCode", back_populates="lock", cascade="all, delete-orphan")
    rfid_cards = relationship("RFIDCard", back_populates="lock", cascade="all, delete-orphan")
    access_logs = relationship("AccessLog", back_populates="lock", cascade="all, delete-orphan")


class AccessCode(Base):
    """PIN access code model."""
    __tablename__ = "access_codes"
    
    id = Column(Integer, primary_key=True, index=True)
    lock_id = Column(Integer, ForeignKey("locks.id"), nullable=False)
    code = Column(String, nullable=False)
    name = Column(String)
    is_active = Column(Boolean, default=True)
    valid_from = Column(DateTime, nullable=True)
    valid_until = Column(DateTime, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow)
    
    # Relationships
    lock = relationship("Lock", back_populates="access_codes")


class RFIDCard(Base):
    """RFID card model."""
    __tablename__ = "rfid_cards"
    
    id = Column(Integer, primary_key=True, index=True)
    lock_id = Column(Integer, ForeignKey("locks.id"), nullable=False)
    card_uid = Column(String, nullable=False, index=True)
    name = Column(String)
    is_active = Column(Boolean, default=True)
    valid_from = Column(DateTime, nullable=True)
    valid_until = Column(DateTime, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow)
    
    # Relationships
    lock = relationship("Lock", back_populates="rfid_cards")


class AccessLog(Base):
    """Access log model."""
    __tablename__ = "access_logs"
    
    id = Column(Integer, primary_key=True, index=True)
    lock_id = Column(Integer, ForeignKey("locks.id"), nullable=False)
    access_type = Column(String, nullable=False)  # 'pin', 'rfid', 'remote'
    access_method = Column(String)  # PIN code or RFID UID
    success = Column(Boolean, nullable=False)
    timestamp = Column(DateTime, default=datetime.utcnow)
    
    # Relationships
    lock = relationship("Lock", back_populates="access_logs")
