import argparse, asyncio, json, sys
from mcp import ClientSession
from mcp.client.streamable_http import streamablehttp_client

def text_of(result):
    out=[]
    for c in getattr(result,'content',[]) or []:
        t=getattr(c,'text',None)
        if t is not None: out.append(t)
    sc=getattr(result,'structuredContent',None)
    return {"text":"\n".join(out),"structured":sc}

async def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--url',default='http://127.0.0.1:8081/mcp')
    ap.add_argument('--list',action='store_true')
    ap.add_argument('tool',nargs='?')
    ap.add_argument('args',nargs='?',default='{}')
    a=ap.parse_args()
    async with streamablehttp_client(a.url) as (r,w,_):
        async with ClientSession(r,w) as s:
            await s.initialize()
            if a.list:
                tools=await s.list_tools()
                print(json.dumps([{'name':t.name,'desc':(t.description or '')[:160]} for t in tools.tools],indent=1))
                return
            res=await s.call_tool(a.tool,json.loads(a.args))
            print(json.dumps(text_of(res),default=str))

asyncio.run(main())
