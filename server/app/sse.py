"""
SSE (Server-Sent Events) broadcaster for real-time updates.
"""
import asyncio
from typing import Dict, Set
from fastapi import Request
import logging

logger = logging.getLogger(__name__)


class SSEBroadcaster:
    """Manages SSE connections and broadcasts events to all connected clients."""
    
    def __init__(self):
        self._clients: Set[asyncio.Queue] = set()
        
    def add_client(self, queue: asyncio.Queue):
        """Add a new SSE client."""
        self._clients.add(queue)
        logger.info(f"SSE client connected. Total clients: {len(self._clients)}")
        
    def remove_client(self, queue: asyncio.Queue):
        """Remove an SSE client."""
        self._clients.discard(queue)
        logger.info(f"SSE client disconnected. Total clients: {len(self._clients)}")
        
    async def broadcast(self, event_type: str, data: dict):
        """Broadcast an event to all connected clients."""
        if not self._clients:
            return
            
        message = {
            "type": event_type,
            "data": data
        }
        
        # Send to all clients
        dead_clients = set()
        for client_queue in self._clients:
            try:
                await client_queue.put(message)
            except Exception as e:
                logger.error(f"Error sending to client: {e}")
                dead_clients.add(client_queue)
        
        # Clean up dead clients
        for dead_client in dead_clients:
            self.remove_client(dead_client)
        
        logger.debug(f"Broadcasted {event_type} to {len(self._clients)} clients")


# Global broadcaster instance
sse_broadcaster = SSEBroadcaster()
