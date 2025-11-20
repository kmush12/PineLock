# PineLock API Examples

This document provides examples for common API operations.

## Base URL

```
http://localhost:8000
```

## 1. Register a Lock

Register a new lock device in the system.

```bash
curl -X POST "http://localhost:8000/api/v1/locks" \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "lock_001",
    "name": "Front Door Lock",
    "location": "Main Entrance"
  }'
```

Response:
```json
{
  "id": 1,
  "device_id": "lock_001",
  "name": "Front Door Lock",
  "location": "Main Entrance",
  "is_online": false,
  "is_locked": true,
  "last_seen": "2024-01-01T00:00:00",
  "created_at": "2024-01-01T00:00:00"
}
```

## 2. List All Locks

```bash
curl "http://localhost:8000/api/v1/locks"
```

## 3. Get Lock Details

```bash
curl "http://localhost:8000/api/v1/locks/1"
```

## 4. Update Lock Information

```bash
curl -X PUT "http://localhost:8000/api/v1/locks/1" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Updated Front Door",
    "location": "Building A - Main Entrance"
  }'
```

## 5. Add PIN Access Code

Add a simple PIN code:

```bash
curl -X POST "http://localhost:8000/api/v1/access-codes" \
  -H "Content-Type: application/json" \
  -d '{
    "lock_id": 1,
    "code": "1234",
    "name": "Guest Code",
    "is_active": true
  }'
```

Add a time-limited PIN code:

```bash
curl -X POST "http://localhost:8000/api/v1/access-codes" \
  -H "Content-Type: application/json" \
  -d '{
    "lock_id": 1,
    "code": "5678",
    "name": "Temporary Guest Code",
    "is_active": true,
    "valid_from": "2024-12-01T00:00:00",
    "valid_until": "2024-12-31T23:59:59"
  }'
```

## 6. List Access Codes for a Lock

```bash
curl "http://localhost:8000/api/v1/locks/1/access-codes"
```

## 7. Update Access Code

Deactivate a code:

```bash
curl -X PUT "http://localhost:8000/api/v1/access-codes/1" \
  -H "Content-Type: application/json" \
  -d '{
    "is_active": false
  }'
```

Update time range:

```bash
curl -X PUT "http://localhost:8000/api/v1/access-codes/2" \
  -H "Content-Type: application/json" \
  -d '{
    "valid_from": "2024-12-15T00:00:00",
    "valid_until": "2024-12-20T23:59:59"
  }'
```

## 8. Delete Access Code

```bash
curl -X DELETE "http://localhost:8000/api/v1/access-codes/1"
```

## 9. Add RFID Card

Add a simple RFID card:

```bash
curl -X POST "http://localhost:8000/api/v1/rfid-cards" \
  -H "Content-Type: application/json" \
  -d '{
    "lock_id": 1,
    "card_uid": "A1B2C3D4",
    "name": "Admin Card",
    "is_active": true
  }'
```

Add a time-limited RFID card:

```bash
curl -X POST "http://localhost:8000/api/v1/rfid-cards" \
  -H "Content-Type: application/json" \
  -d '{
    "lock_id": 1,
    "card_uid": "E5F6G7H8",
    "name": "Maintenance Card",
    "is_active": true,
    "valid_from": "2024-01-01T08:00:00",
    "valid_until": "2024-01-01T18:00:00"
  }'
```

## 10. List RFID Cards for a Lock

```bash
curl "http://localhost:8000/api/v1/locks/1/rfid-cards"
```

## 11. Update RFID Card

```bash
curl -X PUT "http://localhost:8000/api/v1/rfid-cards/1" \
  -H "Content-Type: application/json" \
  -d '{
    "is_active": false
  }'
```

## 12. Delete RFID Card

```bash
curl -X DELETE "http://localhost:8000/api/v1/rfid-cards/1"
```

## 13. Send Lock Command

Unlock:

```bash
curl -X POST "http://localhost:8000/api/v1/locks/1/command" \
  -H "Content-Type: application/json" \
  -d '{
    "action": "unlock"
  }'
```

Lock:

```bash
curl -X POST "http://localhost:8000/api/v1/locks/1/command" \
  -H "Content-Type: application/json" \
  -d '{
    "action": "lock"
  }'
```

## 14. View Access Logs

Get last 100 access attempts:

```bash
curl "http://localhost:8000/api/v1/locks/1/access-logs"
```

Get last 50 access attempts:

```bash
curl "http://localhost:8000/api/v1/locks/1/access-logs?limit=50"
```

Response:
```json
[
  {
    "id": 1,
    "lock_id": 1,
    "access_type": "pin",
    "access_method": "1234",
    "success": true,
    "timestamp": "2024-01-01T12:00:00"
  },
  {
    "id": 2,
    "lock_id": 1,
    "access_type": "rfid",
    "access_method": "A1B2C3D4",
    "success": true,
    "timestamp": "2024-01-01T12:05:00"
  },
  {
    "id": 3,
    "lock_id": 1,
    "access_type": "pin",
    "access_method": "9999",
    "success": false,
    "timestamp": "2024-01-01T12:10:00"
  }
]
```

## 15. Health Check

```bash
curl "http://localhost:8000/health"
```

Response:
```json
{
  "status": "healthy",
  "mqtt_connected": true
}
```

## 16. API Root

```bash
curl "http://localhost:8000/"
```

Response:
```json
{
  "name": "PineLock Server",
  "version": "1.0.0",
  "status": "running"
}
```

## Python Examples

Using Python `requests` library:

```python
import requests

BASE_URL = "http://localhost:8000/api/v1"

# Register a lock
def register_lock(device_id, name, location):
    response = requests.post(
        f"{BASE_URL}/locks",
        json={
            "device_id": device_id,
            "name": name,
            "location": location
        }
    )
    return response.json()

# Add PIN code
def add_pin_code(lock_id, code, name):
    response = requests.post(
        f"{BASE_URL}/access-codes",
        json={
            "lock_id": lock_id,
            "code": code,
            "name": name,
            "is_active": True
        }
    )
    return response.json()

# Unlock door
def unlock_door(lock_id):
    response = requests.post(
        f"{BASE_URL}/locks/{lock_id}/command",
        json={"action": "unlock"}
    )
    return response.json()

# Get access logs
def get_access_logs(lock_id, limit=100):
    response = requests.get(
        f"{BASE_URL}/locks/{lock_id}/access-logs",
        params={"limit": limit}
    )
    return response.json()

# Example usage
if __name__ == "__main__":
    # Register a new lock
    lock = register_lock("lock_001", "Front Door", "Main Entrance")
    print(f"Registered lock: {lock}")
    
    # Add a PIN code
    pin = add_pin_code(lock["id"], "1234", "Admin PIN")
    print(f"Added PIN code: {pin}")
    
    # Unlock the door
    result = unlock_door(lock["id"])
    print(f"Unlock result: {result}")
    
    # Get access logs
    logs = get_access_logs(lock["id"], limit=10)
    print(f"Recent access logs: {logs}")
```

## JavaScript Examples

Using `fetch` API:

```javascript
const BASE_URL = 'http://localhost:8000/api/v1';

// Register a lock
async function registerLock(deviceId, name, location) {
    const response = await fetch(`${BASE_URL}/locks`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            device_id: deviceId,
            name: name,
            location: location
        })
    });
    return response.json();
}

// Add PIN code
async function addPinCode(lockId, code, name) {
    const response = await fetch(`${BASE_URL}/access-codes`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            lock_id: lockId,
            code: code,
            name: name,
            is_active: true
        })
    });
    return response.json();
}

// Unlock door
async function unlockDoor(lockId) {
    const response = await fetch(`${BASE_URL}/locks/${lockId}/command`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            action: 'unlock'
        })
    });
    return response.json();
}

// Get access logs
async function getAccessLogs(lockId, limit = 100) {
    const response = await fetch(
        `${BASE_URL}/locks/${lockId}/access-logs?limit=${limit}`
    );
    return response.json();
}

// Example usage
(async () => {
    try {
        // Register a new lock
        const lock = await registerLock('lock_001', 'Front Door', 'Main Entrance');
        console.log('Registered lock:', lock);
        
        // Add a PIN code
        const pin = await addPinCode(lock.id, '1234', 'Admin PIN');
        console.log('Added PIN code:', pin);
        
        // Unlock the door
        const result = await unlockDoor(lock.id);
        console.log('Unlock result:', result);
        
        // Get access logs
        const logs = await getAccessLogs(lock.id, 10);
        console.log('Recent access logs:', logs);
    } catch (error) {
        console.error('Error:', error);
    }
})();
```

## MQTT Testing

Test MQTT communication using mosquitto clients:

### Subscribe to all topics

```bash
mosquitto_sub -h localhost -t "pinelock/#" -v
```

### Publish lock command

```bash
mosquitto_pub -h localhost \
  -t "pinelock/lock_001/command" \
  -m '{"action":"unlock"}'
```

### Simulate device status update

```bash
mosquitto_pub -h localhost \
  -t "pinelock/lock_001/status" \
  -m '{"is_locked":false,"timestamp":1234567890}'
```

### Simulate access event

```bash
mosquitto_pub -h localhost \
  -t "pinelock/lock_001/access" \
  -m '{"access_type":"pin","access_method":"1234","success":true,"timestamp":1234567890}'
```

## Error Handling

The API returns standard HTTP status codes:

- `200 OK` - Success
- `201 Created` - Resource created successfully
- `204 No Content` - Success with no response body
- `400 Bad Request` - Invalid request data
- `404 Not Found` - Resource not found
- `503 Service Unavailable` - Service temporarily unavailable (e.g., MQTT not connected)

Error response example:
```json
{
  "detail": "Lock not found"
}
```
