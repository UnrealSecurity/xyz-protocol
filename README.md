# Xyz Protocol (tcp/ip)

## C#

### Server example
```csharp
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
};
```

### Client example
```csharp
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
