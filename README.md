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

#### Opcode 종류
| Opcode | Name | Description | Data Configuration (Optional) |
| :---:   | :-: | :-: | :-: |
| 0000 (0x0) | Client Login | 클라이언트 최초 로그인 | nickname (~30 byte) |
| 0001 (0x1) | Client Chat Request | 채팅 요청 | Dest (4 byte), Message (~1012 byte) |
| 1000 (0x8) | Server Chat Relay | 채팅 전달 | Sender(4 byte), Message (~1012 byte) |
| 1001 (0x9) | Server User List | 접속 유저 현황 | Count(4 byte), Userdata(nick(30), sockfd(4), connected(1))\*Count |
| 1111 (0xF) | Server Error ACK | 오류 메시지 | Errno(4 byte), Detail (~ 1012 byte) |


#### 에러 코드

| Errno | Name | Description | Detail(Msg) |
| :---: | :-:  | :-: | :-: |
| 0000 (0x0) | Nickname Not Allowed | 닉네임이 최대글자를 초과했을 경우 (30자) | - |
| 0001 (0x1) | Invaild Destination | 메시지 수신자가 존재하지 않을 경우 | - |
| 0002 (0x2) | User Already Connected | 로그인 시, 해당 닉네임을 가진 사람이 이미 있을 경우 | - |
| 1111 (0xF) | Unknown Error | 알 수 없는 오류 | Message (~1012 byte) |
