# Xyz Network Message Protocol
Xyz library is currently only available for C#. All messages are automatically compressed with Deflate before sending and decompressed upon receiving.

## Installation
Copy XyzSharp folder to your project directory and if it doesn't automatically show in Solution Explorer, click "Show all files" icon in Solution Explorer and you should see it. You can now rightclick it and "Include in project". To allow the use of types in a namespace so that you do not have to qualify the use of a type in that namespace use
```csharp
using XyzSharp;
```

#### ECDiffieHellmanCng not found? (.NET core related issue?)
https://www.nuget.org/packages/System.Security.Cryptography.Cng/

### Server example
```csharp
XyzServer server = new XyzServer(1234);

server.OnConnect = (XyzClient client) =>
{
    Console.WriteLine("[server] Client connected to us");

    client.OnDisconnect = () => {
        Console.WriteLine("[server] Disconnected");
    };

    client.OnMessage = (byte[] data, int type) =>
    {
        Console.WriteLine("[server received] " + type.ToString() + ": " + Encoding.UTF8.GetString(data));
        client.Send("Hello from server!", 2);
    };

    client.Read();
};
```

### Client example
```csharp
XyzClient client = new XyzClient();

client.OnConnect = () => {
    Console.WriteLine("[client] Client connected to server");
};

client.OnDisconnect = () => {
    Console.WriteLine("[client] Disconnected");
};

client.OnMessage = (byte[] data, int type) =>
{
    Console.WriteLine("[client received] " + type.ToString() + ": " + Encoding.UTF8.GetString(data));
};

if (client.Connect("127.0.0.1", 1234))
{
    client.Send("Hello from client!", 1);
}
```
