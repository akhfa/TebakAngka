# TebakAngka

**Build:**

`cmake CMakeLists.txt`

`make`



**Run:**

`./TebakAngka <account_sid> <auth_token>`



**Endpoint:**

```
Method  Path                                    Response        Note

GET     http://localhost:18080/                 Hello cpp

GET     http://localhost:18080/hello/{int}      Hello {int}

POST    http://localhost:18080/message          OK              Webhook from twilio
```
