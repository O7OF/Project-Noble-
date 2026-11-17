import asyncio
from mcp import ClientSession
from mcp.client.streamable_http import streamablehttp_client
async def main():
    async with streamablehttp_client('http://127.0.0.1:8081/mcp') as (r,w,_):
        async with ClientSession(r,w) as s:
            await s.initialize()
            tools=await s.list_tools()
            for t in tools.tools: print(t.name)
asyncio.run(main())
