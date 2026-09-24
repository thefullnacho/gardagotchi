"""Pull the calibration log off the board and save it as a CSV.

Plug the board into this computer, then:

    python3 tools/calib_pull.py              # auto-finds the port
    python3 tools/calib_pull.py /dev/ttyACM0

It also sets the board's clock from this computer, so rows logged after this
point carry real timestamps. (The board has no battery-backed clock, so after a
power cut it only knows "seconds since boot" until you run this again.)

Saves to data/calib-<date>.csv. Nothing on the board is erased.
Note: plugging in over USB may reset the board. That just starts a new boot
number in the log; no data is lost.
"""
import datetime
import glob
import os
import sys
import time

import serial

HERE = os.path.dirname(os.path.abspath(__file__))


def find_port():
    ports = sorted(glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*"))
    if not ports:
        sys.exit("no board found; is it plugged in?")
    return ports[0]


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else find_port()
    s = serial.Serial(baudrate=115200, timeout=2)
    s.port = port
    s.dtr = False  # try not to reset the board when the port opens
    s.rts = False
    s.open()
    with s:
        time.sleep(2.5)  # let it finish booting if opening the port reset it
        s.reset_input_buffer()
        s.write(f"time {int(time.time())}\n".encode())
        s.write(b"dump\n")
        rows, inside, deadline = [], False, time.time() + 120
        while time.time() < deadline:
            raw = s.readline().decode(errors="replace").strip()
            if raw == "---BEGIN---":
                inside = True
            elif raw == "---END---":
                break
            elif inside and raw:
                rows.append(raw)
        else:
            sys.exit("timed out waiting for the log; try again")

    os.makedirs(os.path.join(HERE, "..", "data"), exist_ok=True)
    stamp = datetime.datetime.now().strftime("%Y-%m-%d-%H%M")
    out = os.path.join(HERE, "..", "data", f"calib-{stamp}.csv")
    with open(out, "w") as f:
        f.write("\n".join(rows) + "\n")
    print(f"{len(rows) - 1} rows -> {os.path.relpath(out)}")


if __name__ == "__main__":
    main()
