import logging

from ReedSwitchChessboard.chessboard import RSCb
from utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)

cb = RSCb()
print(cb)
print(cb.reader)
print(cb.readerBitsMatchBoard())
