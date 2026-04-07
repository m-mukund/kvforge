import socket
import time
import threading

# Configuration
HOST = '127.0.0.1'
PORT = 8080
NUM_CLIENTS = 10
REQUESTS_PER_CLIENT = 2000

def client_worker():
    # Create a TCP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        
        for i in range(REQUESTS_PER_CLIENT):
            # Send a SET command
            req = f"SET key{i} val{i}\n".encode('utf-8')
            s.sendall(req)
            s.recv(1024) # Wait for "OK\n"
            
            # Send a GET command
            req = f"GET key{i}\n".encode('utf-8')
            s.sendall(req)
            s.recv(1024) # Wait for value

def run_benchmark():
    print(f"Starting benchmark with {NUM_CLIENTS} concurrent clients...")
    print(f"Each client sending {REQUESTS_PER_CLIENT * 2} requests.")
    
    threads = []
    start_time = time.time()
    
    for _ in range(NUM_CLIENTS):
        t = threading.Thread(target=client_worker)
        threads.append(t)
        t.start()
        
    for t in threads:
        t.join()
        
    end_time = time.time()
    duration = end_time - start_time
    total_requests = NUM_CLIENTS * REQUESTS_PER_CLIENT * 2
    rps = total_requests / duration
    
    print("-" * 30)
    print(f"Total Requests: {total_requests}")
    print(f"Time Taken:     {duration:.2f} seconds")
    print(f"Throughput:     {rps:.2f} requests/second")

if __name__ == "__main__":
    run_benchmark()