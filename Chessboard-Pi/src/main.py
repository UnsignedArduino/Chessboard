import logging
from time import sleep

import chess

from ReedSwitchChessboard.chessboard import RSCb, RSCbState
from utils.logger import create_logger

logger = create_logger(name=__name__, level=logging.DEBUG)

cb = RSCb()

previous_state = None
while True:
    state = cb.update()
    if state != previous_state:
        previous_state = state
        print(state)
        if state in (RSCbState.WaitingForWhiteMove, RSCbState.WaitingForBlackMove):
            print(cb.board)
            b = chess.Board()
            print(b.variation_san(cb.board.move_stack))
    sleep(1)
