import asyncio, json, sys
from mcp import ClientSession
from mcp.client.streamable_http import streamablehttp_client

async def main():
    path=sys.argv[1]
    calls=json.load(open(path,encoding='utf-8'))
    async with streamablehttp_client('http://127.0.0.1:8081/mcp') as (r,w,_):
        async with ClientSession(r,w) as s:
            await s.initialize()
            if calls and calls[0].get('tool')=='__tools__':
                tools=await s.list_tools()
                for t in tools.tools:
                    print("###",t.name)
                    print(json.dumps(t.inputSchema,indent=1)[:3000])
                return
            for c in calls:
                tool=c['tool']; args=c.get('args') or {}
                print("\n===== ",tool,json.dumps(args)[:300])
                try:
                    res=await s.call_tool(tool,args)
                    for item in getattr(res,'content',[]) or []:
                        t=getattr(item,'text',None)
                        if t is not None: print(t[:12000])
                except Exception as e:
                    print("ERROR",repr(e))
asyncio.run(main())
