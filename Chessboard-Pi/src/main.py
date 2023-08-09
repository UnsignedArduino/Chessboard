import logging
from time import sleep

from chessboard import ReedSwitchChessboard
from utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)

cb = ReedSwitchChessboard()
print(cb)

while True:
    if cb.update():
        print(cb)
    sleep(0.1)
