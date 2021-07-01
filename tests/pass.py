def minus(a,b):
    return a-b

def plus(a,b):
    return a+b

def op(a,b,o):
    return o(a,b)

print(op(10,5, minus))
print(op(10,5, plus))
