all:
	gcc -o aws aws.c
	gcc -o serverA serverA.c
	gcc -o serverB serverB.c
	gcc -o serverC serverC.c
	gcc -o monitor monitor.c
	gcc -o client client.c

.PHONY: aws
aws:
	./aws

.PHONY: serverA
serverA:
	./serverA

.PHONY: serverB
serverB:
	./serverB

.PHONY: serverC
serverC:
	./serverC

.PHONY: monitor
monitor:
	./monitor

clean:
	rm -r aws serverA serverB serverC monitor client