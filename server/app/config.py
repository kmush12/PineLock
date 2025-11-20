from pydantic_settings import BaseSettings
from typing import Optional


class Settings(BaseSettings):
    """Application settings loaded from environment variables."""
    
    # MQTT Configuration
    mqtt_broker_host: str = "localhost"
    mqtt_broker_port: int = 1883
    mqtt_username: Optional[str] = None
    mqtt_password: Optional[str] = None
    mqtt_topic_prefix: str = "pinelock"
    
    # Database Configuration
    database_url: str = "sqlite+aiosqlite:///./locks.db"
    
    # API Configuration
    api_host: str = "0.0.0.0"
    api_port: int = 8000
    api_title: str = "PineLock Server"
    api_version: str = "1.0.0"
    
    # Security
    secret_key: str = "your-secret-key-here-change-in-production"
    access_token_expire_minutes: int = 30
    
    class Config:
        env_file = ".env"
        case_sensitive = False


settings = Settings()
