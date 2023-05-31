import asyncio
import websockets
import time
import json
from threading import Thread, Lock
from queue import Queue

NUM_CLIENTS = 10
NUM_MESSAGES = 10
MAX_WAIT_TIME = 0.05  # 50 ms

# TODO: Integration test currently broken
def test_websockets_chat_server():
    server_uri = 'ws://localhost:8080'  # Replace this with your server URI
    clients = [WebsocketClient(server_uri) for _ in range(NUM_CLIENTS)]

    threads = [Thread(target=client.run) for client in clients]
    for thread in threads:
        thread.start()

    time.sleep(2)  # Give some time for all clients to connect

    for i, client in enumerate(clients):
        for j in range(NUM_MESSAGES):
            client.enqueue_message(f'Client {i}: message {j}')

    time.sleep(2)  # Give some time for all messages to be sent

    for client in clients:
        received_count = len(client.received_messages)
        expected_count = NUM_CLIENTS * NUM_MESSAGES
        print(f"Client received: {received_count} / Expected: {expected_count}")
        assert received_count == expected_count, "Some messages were not received"

    print('All messages were successfully received by all clients within the time limit.')


class WebsocketClient:
    def __init__(self, uri):
        self.uri = uri
        self.ws = None
        self.received_messages = []
        self.received_messages_lock = Lock()
        self.start_time = None
        self.messages_to_send = Queue()

    async def connect(self):
        self.ws = await websockets.connect(self.uri)
        self.start_time = time.time()
        print(f'[Client] Connected to {self.uri}')

    async def send_messages(self):
        while True:
            print('waiting for send')
            message = await self.messages_to_send.get()
            await self.ws.send(json.dumps({'message': message}))
            print(f'[Client] Sent: {message}')

    async def receive_message(self):
        while True:
            message = await self.ws.recv()
            message = json.loads(message)['message']
            with self.received_messages_lock:
                self.received_messages.append(message)
            elapsed_time = time.time() - self.start_time
            assert elapsed_time <= MAX_WAIT_TIME, \
                f"Message '{message}' was not received within the time limit"

    async def run_loop(self):
        await self.connect()
        send_task = asyncio.create_task(self.send_messages())
        recv_task = asyncio.create_task(self.receive_message())
        try:
            while True:
                await asyncio.sleep(1)  # Yield control to other tasks
        except asyncio.CancelledError:
            send_task.cancel()
            recv_task.cancel()

    def run(self):
        asyncio.run(self.run_loop())

    def enqueue_message(self, message):
        self.messages_to_send.put(message)


if __name__ == "__main__":
    test_websockets_chat_server()
