st1 = input()
st1=st1[::-1]
st2 = input()
st2=st2[::-1]

a = int(st1, 2)
b = int(st2, 2)
def toBinary(n):
    return format(n, "b")

print(toBinary(a*b))