# RUDP (Reliable UDP)
> UDP의 비신뢰적인 특성(비순차성, 재전송 등)을 극복하기 위해 TCP의 일부 기능을 UDP에 적용하여 신뢰성을 향상시키는 기술을 의미한다. UDP는 어플리케이션에서 그 기능들을 구현해야 한다. 

## 필요한 기능
### 순차성 보장
> 순서가 맞지 않으면 별도의 버퍼에 임시로 넣어놓고, 순서에 맞는 패킷이 오면 처리한다.  
### 재전송 기능
> Sender는 보낸 패킷의 번호를 별도의 버퍼에 임시로 저장해놓고, Receiver로부터 Ack 패킷이 수신하면 해당 패킷을 버퍼에서 삭제한다. Ack 패킷이 오기 전까지는 주기적으로 패킷을 재전송한다. 전송주기는 빨라서도 너무 느려서도 안된다.
