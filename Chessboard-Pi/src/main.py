import logging

from chessboard import ReedSwitchChessboard
from utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)

cb = ReedSwitchChessboard()
cb.update()
print(cb)
