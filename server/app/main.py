from fastapi import FastAPI
from contextlib import asynccontextmanager
import logging

from app.config import settings
from app.database import init_db
from app.routes import router
from app.mqtt_client import mqtt_client
from app.mqtt_handlers import setup_mqtt_handlers

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)

logger = logging.getLogger(__name__)


@asynccontextmanager
async def lifespan(app: FastAPI):
    """Lifespan context manager for startup and shutdown events."""
    # Startup
    logger.info("Starting PineLock Server...")
    
    # Initialize database
    await init_db()
    logger.info("Database initialized")
    
    # Connect to MQTT broker and setup handlers
    try:
        mqtt_client.connect()
        setup_mqtt_handlers(mqtt_client)
        logger.info("MQTT client connected and handlers registered")
    except Exception as e:
        logger.error(f"Failed to setup MQTT: {e}")
    
    yield
    
    # Shutdown
    logger.info("Shutting down PineLock Server...")
    mqtt_client.disconnect()


# Create FastAPI app
app = FastAPI(
    title=settings.api_title,
    version=settings.api_version,
    lifespan=lifespan
)

# Include routes
app.include_router(router, prefix="/api/v1")


@app.get("/")
async def root():
    """Root endpoint."""
    return {
        "name": settings.api_title,
        "version": settings.api_version,
        "status": "running"
    }


@app.get("/health")
async def health():
    """Health check endpoint."""
    return {
        "status": "healthy",
        "mqtt_connected": mqtt_client.is_connected
    }
