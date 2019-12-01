import requests

url = "https://prom.model.amoy.ai/api/v1/alerts"
# print("==> Trying Sync")
# response = requests.get(url, timeout=5)
# print(response.status_code, response.text)


import aiohttp
import asyncio
import aiohttp.client_exceptions

import datetime

async def fetch(session, url):
    try:
        async with session.get(url, timeout=5) as response:
            return await response.text()
    except aiohttp.client_exceptions.ClientConnectionError as err:
        print(err)
        raise err
        return None

async def main():
    async with aiohttp.ClientSession() as session:
        status = await fetch(session, url)
        print(type(status))

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    print(datetime.datetime.now())
    print("==> Trying ASync")
    loop.run_until_complete(main())
    print(datetime.datetime.now())