import asyncio
import uvloop

asyncio.set_event_loop_policy(uvloop.EventLoopPolicy())

async def handle_echo(reader, writer):
    print('read')
    data = await reader.read(1024)
    response = str(len(data)).encode('utf-8')

    #print(response)
    writer.write(response)
    await writer.drain()

    #print("close")
    writer.close()

print('Starting to UV listen')
loop = asyncio.get_event_loop()
coro = asyncio.start_server(handle_echo, '0.0.0.0', 1234, loop=loop)
server = loop.run_until_complete(coro)
loop.run_forever()
