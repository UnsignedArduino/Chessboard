import logging

from chessboard.chessboard import ReedSwitchChessboard
from utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)

cb = ReedSwitchChessboard()
print(cb)
print(cb.reader)
print(cb.readerBitsMatchBoard())
