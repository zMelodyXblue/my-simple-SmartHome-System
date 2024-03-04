gcc -o ./bin/client  client.c ./include/common.c ./include/client_recv.c ./include/chat_ui.c -I./include -lpthread -lncursesw -lm -D_D
