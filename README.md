# xyz-protocol
Xyz Protocol (TCP/IP)

### Server example
```csharp
server.OnConnect = (XyzSession session) =>
{
    session.OnMessage = (byte[] data, int type) =>
    {
        if (data.Length > 64)
            data = Encoding.UTF8.GetBytes("[payload size = " + data.Length.ToString() + "]");

        Console.WriteLine("[server received] " + type.ToString() + ": " + Encoding.UTF8.GetString(data));
        session.Send("Hello from server!", 2);
    };
};
```

### Client example
```csharp
client.OnConnect = (session) => {
    Console.WriteLine("Client connected to server");
};
client.OnMessage = (byte[] data, int type) =>
{
    if (data.Length > 64)
        data = Encoding.UTF8.GetBytes("[payload size = " + data.Length.ToString() + "]");

    Console.WriteLine("[client received] " + type.ToString() + ": " + Encoding.UTF8.GetString(data));
};
if (client.Connect("127.0.0.1", 1234))
{
    client.Send("Hello from client!", 1);
}
```
