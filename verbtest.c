#include <stdio.h>
#include <stdlib.h>
#include "verb.c"

int main()
{
	char msg[50] = "CREATP name \r\n";

	Verb *v1 = decode_Verb(msg);

	if (v1)
		printf("%s %s %s\n", v1->name, v1->args[0], v1-> args[1]);

	char *message = encode_Verb(v1);

	printf("%s\n", message);

	destroy_Verb(v1);
	Free(message);

	return 0;
}