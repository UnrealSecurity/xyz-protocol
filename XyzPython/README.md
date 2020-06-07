## Installation
Copy XyzPython to your project's directory and install all required modules listed in file called requirements.txt
```bash
sudo pip install -r requirements.txt
```

### Client example
```python
from XyzPython.XyzClient import XyzClient
from XyzPython.XyzMessageBuilder import XyzMessageBuilder
from XyzPython.XyzMessageDecoder import XyzMessageDecoder
from XyzPython.XyzKeyExchange import XyzKeyExchange
from XyzPython.XyzMessage import XyzMessage
from XyzPython.XyzUtils import XyzUtils

builder = XyzMessageBuilder()
builder.add(data=b"Hello world!")
builder.add(data=["Hello", "world!"])

client = XyzClient()

def onConnect(client):
    messages = builder.compileMessages()
    client.send(messages=messages, msgid=1)

def onMessage(client, data, msgid):
    # echo server sent our message back to us
    messages = XyzMessageDecoder(data).decode()
    print(messages[1].strings())
    pass

client.onConnect = onConnect
client.onMessage = onMessage

client.connect("127.0.0.1", 1234)
```