from subprocess import Popen, PIPE

def test(socket_count):
    print('-' * 70)
    print(socket_count)
    print('-' * 70)
    p = Popen([r"Debug\SocketTest.exe", str(socket_count)])
    output = p.communicate()[0]
    selects_per_second = p.returncode
    print(selects_per_second)
    return selects_per_second

out = open('results.csv', 'w')
for socks in reversed([1, 10, 100, 1000, 10000]):
    selects_per_second = test(socks)
    out.write('%d,%d\n' % (socks, selects_per_second))

