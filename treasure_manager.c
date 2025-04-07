#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#define STRING_SIZE 256
#define PASSWORD "123456"
#define HUNT_FILE "hunt.log"
#define TREASURE_FILE "treasure.dat"

typedef struct {
    char id[STRING_SIZE];
    char username[STRING_SIZE];
    float latitude;
    float longitude;
    char clue[STRING_SIZE];
    int value;
} Treasure;

void logAction(char* hunt_id, char* action) {
    char file_path[STRING_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, HUNT_FILE);

    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return;
    }

    dprintf(fd, "%s\n", action);
    close(fd);

    char symlink_name[STRING_SIZE];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);
    symlink(file_path, symlink_name);
}

void addTreasure(char* hunt_id) {
    mkdir(hunt_id, 0755);

    char file_path[STRING_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return;
    }

    Treasure t;

    printf("Treasure ID: ");
    fgets(t.id, STRING_SIZE, stdin);
    t.id[strcspn(t.id, "\n")] = '\0';

    printf("Username: ");
    fgets(t.username, STRING_SIZE, stdin);
    t.username[strcspn(t.username, "\n")] = '\0';

    printf("Latitude: ");
    scanf("%f", &t.latitude);
    getchar();

    printf("Longitude: ");
    scanf("%f", &t.longitude);
    getchar(); 

    printf("Clue: ");
    fgets(t.clue, STRING_SIZE, stdin);
    t.clue[strcspn(t.clue, "\n")] = '\0';

    printf("Value: ");
    scanf("%d", &t.value);
    getchar(); 

    write(fd, &t, sizeof(Treasure));
    close(fd);

    char action[STRING_SIZE];
    snprintf(action, sizeof(action), "Added treasure %s.", t.id);
    logAction(hunt_id, action);

    printf("Treasure added successfully.\n");
}

void viewTreasure(char* hunt_id, char* target_id) {
    char file_path[STRING_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return;
    }

    Treasure t;
    int found = 0;

    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (strcmp(t.id, target_id) == 0) {
            printf("ID: %s\n", t.id);
            printf("User: %s\n", t.username);
            printf("Lat: %.2f\n", t.latitude);
            printf("Long: %.2f\n", t.longitude);
            printf("Clue: %s\n", t.clue);
            printf("Value: %d\n", t.value);
            found = 1;
            break;
        }
    }

    close(fd);

    if (!found) {
        printf("Treasure %s not found.\n", target_id);
    }

    char action[STRING_SIZE];
    snprintf(action, sizeof(action), "Viewed treasure %s.", target_id);
    logAction(hunt_id, action);
}

void removeTreasure(char* hunt_id, char* target_id) {
    char file_path[STRING_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return;
    }

    char temp_path[STRING_SIZE];
    snprintf(temp_path, sizeof(temp_path), "%s/temp.dat", hunt_id);
    int temp_fd = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        close(fd);
        return;
    }

    Treasure t;
    int removed = 0;

    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (strcmp(t.id, target_id) != 0) {
            write(temp_fd, &t, sizeof(Treasure));
        } else {
            removed = 1;
        }
    }

    close(fd);
    close(temp_fd);

    if (removed) {
        rename(temp_path, file_path);
        char action[STRING_SIZE];
        snprintf(action, sizeof(action), "Removed treasure %s.", target_id);
        logAction(hunt_id, action);
        printf("Treasure %s removed successfully.\n", target_id);
    } else {
        unlink(temp_path);
        printf("Treasure %s not found.\n", target_id);
    }
}

void listTreasures(char* hunt_id) {
    char file_path[STRING_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);

    struct stat st;
    if (stat(file_path, &st) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return;
    }

    printf("Hunt: %s\n", hunt_id);
    printf("Size: %ld bytes\n", st.st_size);
    printf("Last modified: %s\n", ctime(&st.st_mtime));

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return;
    }

    Treasure treasure;
    while (read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("ID: %s, User: %s, Lat: %.2f, Long: %.2f, Value: %d\n",
            treasure.id, treasure.username, treasure.latitude, treasure.longitude, treasure.value);
    }
    close(fd);

    char action[STRING_SIZE];
    snprintf(action, sizeof(action), "Listed treasures from hunt %s.", hunt_id);
    logAction(hunt_id, action);
}

void removeHunt(char* hunt_id) {
    char file_path[STRING_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);
    unlink(file_path);

    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, HUNT_FILE);
    unlink(file_path);

    rmdir(hunt_id);

    char symlink_name[STRING_SIZE];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);

    printf("Hunt %s removed.\n", hunt_id);
}

void resetData(char* password) {
    if (strcmp(password, PASSWORD) == 0) {
        printf("Are you sure?\n");
        printf("Answer (Yes/No): ");
        char line[STRING_SIZE];
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "Yes") == 0) {
            system("rm -f logged_hunt-* && rm -rf */hunt.log */treasure.dat */temp.dat */ && rm -rf */");
            printf("Data successfully erased.\n");
        } else {
            printf("Reset process stopped.\n");
        }
    } else {
        printf("Invalid password!\n");
    }
}

void printUsage(char* arg) {
    printf("Usage:\n");
    printf("%s --add <hunt_id>\n", arg);
    printf("%s --list <hunt_id>\n", arg);
    printf("%s --view <hunt_id> <treasure_id>\n", arg);
    printf("%s --remove_treasure <hunt_id> <treasure_id>\n", arg);
    printf("%s --remove_hunt <hunt_id>\n", arg);
    printf("%s --reset <password>\n", arg);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--add") == 0) {
        addTreasure(argv[2]);
    } else if (strcmp(argv[1], "--list") == 0) {
        listTreasures(argv[2]);
    } else if (strcmp(argv[1], "--view") == 0 && argc >= 4) {
        viewTreasure(argv[2], argv[3]);
    } else if (strcmp(argv[1], "--remove_treasure") == 0 && argc >= 4) {
        removeTreasure(argv[2], argv[3]);
    } else if (strcmp(argv[1], "--remove_hunt") == 0) {
        removeHunt(argv[2]);
    } else if (strcmp(argv[1], "--reset") == 0) {
        resetData(argv[2]);
    } else {
        printUsage(argv[0]);
    }

    return 0;
}
