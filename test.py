import socket
from itertools import chain

UDP_IP = "127.0.0.1"
UDP_PORT = 10001

PLAYER_COUNT = 100

MI = [el for tup in list(zip(range(int(PLAYER_COUNT/2)),
                             range(int(PLAYER_COUNT/2)))) for el in tup]

SS = [socket.socket(socket.AF_INET,
                    socket.SOCK_DGRAM) for _ in range(PLAYER_COUNT)]

# join the sockets to a match
for s in SS:
    s.sendto(b"join", (UDP_IP, UDP_PORT))
    data, addr = s.recvfrom(10001)
    # print(data)


def play_string(move):
    r, c = move
    return b" p" + str(r).encode() + b"-" + str(c).encode()


def play_round(moves):
    one, two = moves
    one = play_string(one)
    two = play_string(two)


    # make the first move
    for idx in range(0, PLAYER_COUNT, 2):
        print(idx, len(MI), len(SS))
        SS[idx].sendto(b"move  m" + str(MI[idx]).encode() + one, (UDP_IP, UDP_PORT))
        mc, addr = SS[idx].recvfrom(10001)
        print(mc)


    # make first move second player
    for idx in range(1, PLAYER_COUNT, 2):
        om, addr = SS[idx].recvfrom(10001)
        print(om)
        SS[idx].sendto(b"move m" + str(MI[idx]).encode() + two, (UDP_IP, UDP_PORT))
        mc, addr = SS[idx].recvfrom(10001)
        print(mc)

    # get player one conformation
    for idx in range(0, PLAYER_COUNT, 2):
        om, addr = SS[idx].recvfrom(10001)
        print(om)

MOVES = [
    ((0, 0),
     (2, 0)),

    ((0, 1),
     (2, 1)),

    ((0, 2),
     (2, 2)),
]

[play_round(moves) for moves in MOVES]
