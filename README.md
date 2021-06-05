# Network-programming-project

## Protocol 정의

### send_CM_info : 멀티캐스트 전송용 프로토콜
* opcode (4 byte), default 1000
* port (4 byte)
* ip (20 byte), char[20]

### request : 클라이언트-서버 통신용 프로토콜
* opcode (4 byte), 0000~1111
* data length (4 byte)
* data (~1016 byte)

| Opcode | Name | Description | Data Configuration (Optional) |
| :---:   | :-: | :-: | :-: |
| 0000 (0x0) | Client Login | 클라이언트 최초 로그인 | nickname (~30 byte) |
| 0001 (0x1) | Client Chat Request | 채팅 요청 | Dest (4 byte), Message (~1012 byte) |
| 1000 (0x8) | Server Chat Relay | 채팅 전달 | Sender(4 byte), Message (~1012 byte) |
