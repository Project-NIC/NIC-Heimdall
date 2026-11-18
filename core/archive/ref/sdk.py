"""Where NIC-SDK's Python cores are found: imported first by hnrc and hmlf."""

import os
import sys

# NIC-SDK beside this repository, or where NIC_SDK points: MLF's and NRC's cores.
sys.path.insert(0, os.path.join(os.environ.get("NIC_SDK") or os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "..", "NIC-SDK"), "tools"))
