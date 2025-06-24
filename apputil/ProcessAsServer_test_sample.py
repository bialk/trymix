import time
import sys
import traceback
import json

def exec_command():
    time.sleep(1)
    return "command_a_completed"

def send_reply(reply):
    sys.stdout.write("eof-json"+json.dumps(reply)+"eof-json")

if __name__ == "__main__":

    print("Initializing...")
    # do initialization before listening commands
    time.sleep(1)
    try:
        while True:
            line = sys.stdin.readline().strip()
            request = json.loads(line)
            command = request["command"]
            if command == "command_a":
                request["result"]=exec_command()
                send_reply(request)
            elif command == "exit":
                request["result"]="output"
                send_reply(request)
                quit(0)
            elif command == "crash":
                5/0 # zero division for crash
                print("{}:{}:{}\n".format("command", line, time.time()))
                quit(0)
            else:
                print("{}:{}:{}\n".format("unknown command", line, time.time()))

    except Exception:
        error_msg = traceback.format_exc()
        print(error_msg)  # Prints stack trace
