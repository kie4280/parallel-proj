import chess
import chess.engine
import chess.svg
from cairosvg import svg2png
# import cv2

SHOW_IMG = 1
P1_ROUND_TIME = 1
P2_ROUND_TIME = 5
ITERRATION = 100

stockfish = chess.engine.SimpleEngine.popen_uci(r"engines/stockfish_14.1_linux_x64_avx2")
pthread_MCTS = chess.engine.SimpleEngine.popen_uci(r"build/main.out")
#cuda_MCTS = chess.engine.SimpleEngine.popen_uci(r"path")

player1 = stockfish
player2 = pthread_MCTS


p1_win_cnt = 0
p2_win_cnt = 0
drawn_cnt = 0
cur_game = 0
board = chess.Board()
while cur_game < ITERRATION:
    cur_game += 1
    board.reset()
    while 1:
        result = player1.play(board, chess.engine.Limit(time=P1_ROUND_TIME))
        board.push(result.move)
        if SHOW_IMG:
            drawing = chess.svg.board(board, size=350, lastmove = result.move)  
            svg2png(bytestring=drawing,write_to='output.png')
            # img = cv2.imread('output.png')
            # cv2.imshow('img', img)
            # cv2.waitKey(33)
        if board.is_game_over():
            winner = board.outcome().winner
            break

        result = player2.play(board, chess.engine.Limit(time=P2_ROUND_TIME))
        board.push(result.move)
        if SHOW_IMG:
            drawing = chess.svg.board(board, size=350, lastmove = result.move)  
            svg2png(bytestring=drawing,write_to='output.png')
            # img = cv2.imread('output.png')
            # cv2.imshow('img', img)
            # cv2.waitKey(33)
        if board.is_game_over():
            winner = board.outcome().winner
            break
    if winner == None:
        drawn_cnt += 1
    elif winner == True:
        p1_win_cnt += 1
    else:
        p2_win_cnt += 1
    print(f"current epoch: {cur_game}/{ITERRATION} result: {p1_win_cnt}-{p2_win_cnt}, Drawn: {drawn_cnt}")

    

player1.quit()
player2.quit()
exit()