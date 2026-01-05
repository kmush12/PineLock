import paho.mqtt.client as mqtt
import json
import logging
import asyncio
import inspect
import os
from typing import Callable, Optional
from concurrent.futures import ThreadPoolExecutor
from app.config import settings

logger = logging.getLogger(__name__)


class MQTTClient:
    """MQTT client for communicating with lock devices."""
    
    def __init__(self):
        self.client: Optional[mqtt.Client] = None
        self.message_handlers = {}
        self.is_connected = False
        self.executor = ThreadPoolExecutor(max_workers=5)
    
    def connect(self):
        """Connect to MQTT broker."""
        try:
            # Use unique client ID per process to avoid conflicts with uvicorn reload
            client_id = f"pinelock_server_{os.getpid()}"
            self.client = mqtt.Client(client_id=client_id)
            
            # Set callbacks
            self.client.on_connect = self._on_connect
            self.client.on_disconnect = self._on_disconnect
            self.client.on_message = self._on_message
            
            # Set credentials if provided
            if settings.mqtt_username and settings.mqtt_password:
                self.client.username_pw_set(
                    settings.mqtt_username,
                    settings.mqtt_password
                )
            
            # Connect to broker
            self.client.connect(
                settings.mqtt_broker_host,
                settings.mqtt_broker_port,
                keepalive=60
            )
            
            # Start network loop in background
            self.client.loop_start()
            
            logger.info(f"Connecting to MQTT broker at {settings.mqtt_broker_host}:{settings.mqtt_broker_port}")
            
        except Exception as e:
            logger.error(f"Failed to connect to MQTT broker: {e}")
            raise
    
    def disconnect(self):
        """Disconnect from MQTT broker."""
        if self.client:
            self.client.loop_stop()
            self.client.disconnect()
            self.executor.shutdown(wait=True)
            logger.info("Disconnected from MQTT broker")
    
    def _on_connect(self, client, userdata, flags, rc):
        """Callback for when client connects to broker."""
        if rc == 0:
            self.is_connected = True
            logger.info("Connected to MQTT broker successfully")
            
            # Subscribe to relevant topics
            topics = [
                f"{settings.mqtt_topic_prefix}/+/status",
                f"{settings.mqtt_topic_prefix}/+/access",
                f"{settings.mqtt_topic_prefix}/+/heartbeat",
                f"{settings.mqtt_topic_prefix}/+/sync",
                f"{settings.mqtt_topic_prefix}/+/alert"
            ]
            for topic in topics:
                self.client.subscribe(topic)
                logger.info(f"Subscribed to topic: {topic}")
        else:
            logger.error(f"Failed to connect to MQTT broker with code: {rc}")
    
    def _on_disconnect(self, client, userdata, rc):
        """Callback for when client disconnects from broker."""
        self.is_connected = False
        if rc != 0:
            logger.warning(f"Unexpected MQTT disconnect with code: {rc}")
        else:
            logger.info("Disconnected from MQTT broker")
    
    def _run_async_handler(self, handler, device_id, data):
        """Run async handler in a new event loop."""
        try:
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)
            try:
                loop.run_until_complete(handler(device_id, data))
            finally:
                loop.close()
        except Exception as e:
            logger.error(f"Error running async handler: {e}", exc_info=True)
    
    def _on_message(self, client, userdata, msg):
        """Callback for when a message is received."""
        try:
            topic = msg.topic
            payload = msg.payload.decode()
            logger.debug(f"Received message on topic {topic}: {payload}")
            
            # Parse topic to get device_id and message type
            parts = topic.split('/')
            if len(parts) >= 3:
                device_id = parts[1]
                message_type = parts[2]
                
                # Call registered handlers
                handler_key = f"{message_type}"
                if handler_key in self.message_handlers:
                    try:
                        data = json.loads(payload)
                        handler = self.message_handlers[handler_key]
                        # Check if handler is async and run it appropriately
                        if inspect.iscoroutinefunction(handler):
                            # Run async handler in thread pool
                            self.executor.submit(self._run_async_handler, handler, device_id, data)
                        else:
                            handler(device_id, data)
                    except json.JSONDecodeError:
                        logger.error(f"Failed to parse JSON payload: {payload}")
                    except Exception as e:
                        logger.error(f"Error in message handler: {e}")
        
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}")
    
    def register_handler(self, message_type: str, handler: Callable):
        """Register a handler for a specific message type."""
        self.message_handlers[message_type] = handler
        logger.info(f"Registered handler for message type: {message_type}")
    
    def publish(self, device_id: str, message_type: str, payload: dict):
        """Publish a message to a device."""
        if not self.client or not self.is_connected:
            logger.error("Cannot publish: MQTT client not connected")
            return False
        
        topic = f"{settings.mqtt_topic_prefix}/{device_id}/{message_type}"
        try:
            message = json.dumps(payload)
            result = self.client.publish(topic, message, qos=1)
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                logger.info(f"Published to {topic}: {message}")
                return True
            else:
                logger.error(f"Failed to publish to {topic}")
                return False
        except Exception as e:
            logger.error(f"Error publishing message: {e}")
            return False
    
    def send_lock_command(self, device_id: str, action: str):
        """Send lock/unlock command to a device."""
        return self.publish(device_id, "command", {"action": action})
    
    def request_sync(self, device_id: str):
        """Request device to sync its access codes and RFID cards."""
        return self.publish(device_id, "sync", {"request": "sync"})


# Global MQTT client instance
mqtt_client = MQTTClient()
