def rec(b):
    if b > 0:
        return rec(b-1)
    return 1337

rec(10)