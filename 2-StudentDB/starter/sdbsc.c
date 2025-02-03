#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // open()
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>     // close(), lseek(), read(), write()
#include <stdbool.h>

// Include header files that define:
//   - student_t, EMPTY_STUDENT_RECORD
//   - DB_FILE, TMP_DB_FILE, STUDENT_RECORD_SIZE, etc.
//   - Message strings: M_ERR_DB_OPEN, M_STD_ADDED, M_ERR_DB_ADD_DUP, M_ERR_DB_READ, M_ERR_DB_WRITE,
//     M_STD_DEL_MSG, M_STD_NOT_FND_MSG, M_DB_RECORD_CNT, M_DB_EMPTY, STUDENT_PRINT_HDR_STRING,
//     STUDENT_PRINT_FMT_STRING, M_ERR_STD_PRINT, M_DB_COMPRESSED_OK, M_ERR_DB_CREATE, etc.
//   - Return codes: NO_ERROR, ERR_DB_FILE, ERR_DB_OP, SRCH_NOT_FOUND, EXIT_FAIL_ARGS, EXIT_FAIL_DB, EXIT_OK.
#include "db.h"
#include "sdbsc.h"

/*
 *  open_db
 *      dbFile:  name of the database file
 *      should_truncate:  indicates if opening the file also empties it
 *
 *  Returns:
 *      File descriptor on success, or ERR_DB_FILE on failure.
 *
 *  Console:
 *      On error, prints M_ERR_DB_OPEN.
 */
int open_db(char *dbFile, bool should_truncate) {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    int flags = O_RDWR | O_CREAT;
    if (should_truncate)
        flags |= O_TRUNC;
    int fd = open(dbFile, flags, mode);
    if (fd == -1) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }
    return fd;
}

/*
 *  get_student
 *      fd:  Linux file descriptor
 *      id:  Student ID to retrieve
 *      s:   Pointer (allocated by caller) to store the student record
 *
 *  Returns:
 *      NO_ERROR       if the student record is found (and copied into *s)
 *      SRCH_NOT_FOUND if the slot is empty (all zeros)
 *      ERR_DB_FILE    if a file I/O error occurs
 *
 *  Console:
 *      No console output.
 */
int get_student(int fd, int id, student_t *s) {
    off_t offset = id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1)
        return ERR_DB_FILE;
    char buffer[STUDENT_RECORD_SIZE];
    ssize_t bytes_read = read(fd, buffer, STUDENT_RECORD_SIZE);
    if (bytes_read == -1)
        return ERR_DB_FILE;
    if (bytes_read != STUDENT_RECORD_SIZE)
        memset(buffer, 0, STUDENT_RECORD_SIZE);
    char zeroBuffer[STUDENT_RECORD_SIZE];
    memset(zeroBuffer, 0, STUDENT_RECORD_SIZE);
    if (memcmp(buffer, zeroBuffer, STUDENT_RECORD_SIZE) == 0)
        return SRCH_NOT_FOUND;
    memcpy(s, buffer, sizeof(student_t)); // copy the student_t portion
    return NO_ERROR;
}

/*
 *  add_student
 *      fd:     Linux file descriptor
 *      id:     Student ID (within allowed range)
 *      fname:  First name
 *      lname:  Last name
 *      gpa:    GPA as an integer (e.g. 345 for 3.45)
 *
 *  Checks if a record already exists at the computed offset.
 *  If the slot is empty, writes the new record.
 *
 *  Returns:
 *      NO_ERROR       on success.
 *      ERR_DB_FILE    on file I/O error.
 *      ERR_DB_OP      if a duplicate record exists.
 *
 *  Console:
 *      On success: prints M_STD_ADDED (with the student ID)
 *      On duplicate: prints M_ERR_DB_ADD_DUP (with the student ID)
 */
int add_student(int fd, int id, char *fname, char *lname, int gpa) {
    off_t offset = id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    char buffer[STUDENT_RECORD_SIZE];
    ssize_t read_bytes = read(fd, buffer, STUDENT_RECORD_SIZE);
    if (read_bytes == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    char zeroBuffer[STUDENT_RECORD_SIZE];
    memset(zeroBuffer, 0, STUDENT_RECORD_SIZE);
    if (read_bytes == STUDENT_RECORD_SIZE && memcmp(buffer, zeroBuffer, STUDENT_RECORD_SIZE) != 0) {
        printf(M_ERR_DB_ADD_DUP, id);
        return ERR_DB_OP;
    }
    // Prepare the new student record.
    student_t new_student;
    new_student.id = id;
    strncpy(new_student.fname, fname, sizeof(new_student.fname)-1);
    new_student.fname[sizeof(new_student.fname)-1] = '\0';
    strncpy(new_student.lname, lname, sizeof(new_student.lname)-1);
    new_student.lname[sizeof(new_student.lname)-1] = '\0';
    new_student.gpa = gpa;
    // Build a write buffer of STUDENT_RECORD_SIZE bytes.
    char writeBuffer[STUDENT_RECORD_SIZE];
    memset(writeBuffer, 0, STUDENT_RECORD_SIZE);
    memcpy(writeBuffer, &new_student, sizeof(student_t));
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    ssize_t written = write(fd, writeBuffer, STUDENT_RECORD_SIZE);
    if (written != STUDENT_RECORD_SIZE) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    printf(M_STD_ADDED, id);
    return NO_ERROR;
}

/*
 *  del_student
 *      fd:  Linux file descriptor
 *      id:  Student ID to delete
 *
 *  If the student exists, writes an all-zero record over that slot.
 *
 *  Returns:
 *      NO_ERROR       on success.
 *      ERR_DB_FILE    on file I/O error.
 *      ERR_DB_OP      if the student is not found.
 *
 *  Console:
 *      On success: prints M_STD_DEL_MSG (with the student ID)
 *      If student not found: prints M_STD_NOT_FND_MSG (with the student ID)
 */
int del_student(int fd, int id) {
    student_t dummy;
    int rc = get_student(fd, id, &dummy);
    if (rc == SRCH_NOT_FOUND) {
        printf(M_STD_NOT_FND_MSG, id);
        return ERR_DB_OP;
    } else if (rc != NO_ERROR) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    off_t offset = id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    char zeroBuffer[STUDENT_RECORD_SIZE];
    memset(zeroBuffer, 0, STUDENT_RECORD_SIZE);
    ssize_t written = write(fd, zeroBuffer, STUDENT_RECORD_SIZE);
    if (written != STUDENT_RECORD_SIZE) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    printf(M_STD_DEL_MSG, id);
    return NO_ERROR;
}

/*
 *  count_db_records
 *      fd:  Linux file descriptor
 *
 *  Reads through the entire database and counts non-empty records.
 *
 *  Returns:
 *      The count of valid records on success,
 *      ERR_DB_FILE on a file I/O error.
 *
 *  Console:
 *      If count > 0: prints M_DB_RECORD_CNT (with the count)
 *      If count == 0: prints M_DB_EMPTY.
 */
int count_db_records(int fd) {
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    int count = 0;
    char buffer[STUDENT_RECORD_SIZE];
    char zeroBuffer[STUDENT_RECORD_SIZE];
    memset(zeroBuffer, 0, STUDENT_RECORD_SIZE);
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, STUDENT_RECORD_SIZE)) == STUDENT_RECORD_SIZE) {
        if (memcmp(buffer, zeroBuffer, STUDENT_RECORD_SIZE) != 0)
            count++;
    }
    if (bytes_read == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    if (count == 0)
        printf(M_DB_EMPTY);
    else
        printf(M_DB_RECORD_CNT, count);
    return count;
}

/*
 *  print_db
 *      fd:  Linux file descriptor
 *
 *  Reads through the entire database and prints all non-empty student records.
 *  Prints a header (using STUDENT_PRINT_HDR_STRING) when the first valid record is found.
 *
 *  Returns:
 *      NO_ERROR on success,
 *      ERR_DB_FILE on file I/O error.
 *
 *  Console:
 *      If records exist, prints the header and each record (using STUDENT_PRINT_FMT_STRING).
 *      Otherwise, prints M_DB_EMPTY.
 */
int print_db(int fd) {
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    bool header_printed = false;
    bool record_found = false;
    char buffer[STUDENT_RECORD_SIZE];
    char zeroBuffer[STUDENT_RECORD_SIZE];
    memset(zeroBuffer, 0, STUDENT_RECORD_SIZE);
    while (read(fd, buffer, STUDENT_RECORD_SIZE) == STUDENT_RECORD_SIZE) {
        if (memcmp(buffer, zeroBuffer, STUDENT_RECORD_SIZE) != 0) {
            if (!header_printed) {
                printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST NAME", "LAST_NAME", "GPA");
                header_printed = true;
            }
            student_t temp;
            memcpy(&temp, buffer, sizeof(student_t));
            float calcGpa = temp.gpa / 100.0;
            printf(STUDENT_PRINT_FMT_STRING, temp.id, temp.fname, temp.lname, calcGpa);
            record_found = true;
        }
    }
    if (!record_found)
        printf(M_DB_EMPTY);
    return NO_ERROR;
}

/*
 *  print_student
 *      s:  Pointer to a student_t structure to print.
 *
 *  If s is NULL or s->id is zero, prints an error message.
 *  Otherwise, prints a header and then the student record.
 *
 *  Returns: Nothing.
 *
 *  Console:
 *      Either prints M_ERR_STD_PRINT or prints the header followed by the record.
 */
void print_student(student_t *s) {
    if (s == NULL || s->id == 0) {
        printf(M_ERR_STD_PRINT);
        return;
    }
    printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST NAME", "LAST_NAME", "GPA");
    float calcGpa = s->gpa / 100.0;
    printf(STUDENT_PRINT_FMT_STRING, s->id, s->fname, s->lname, calcGpa);
}

/*
 *  compress_db    [EXTRA CREDIT]
 *      fd:  File descriptor of the active (original) database.
 *
 *  This function “compresses” the database by creating a new temporary file
 *  that contains only valid (non‑empty) student records. It then renames the
 *  temporary file to become the primary database file and re‑opens it.
 *
 *  Returns:
 *      The file descriptor of the new (compressed) database file on success,
 *      or ERR_DB_FILE on failure.
 *
 *  Console:
 *      On success, prints M_DB_COMPRESSED_OK.
 *      On failure, prints the appropriate error message.
 */
int compress_db(int fd) {
    // Rewind the original file.
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    // Open the temporary file.
    int tmp_fd = open(TMP_DB_FILE, O_RDWR | O_CREAT | O_TRUNC,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (tmp_fd == -1) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }
    char buffer[STUDENT_RECORD_SIZE];
    char zeroBuffer[STUDENT_RECORD_SIZE];
    memset(zeroBuffer, 0, STUDENT_RECORD_SIZE);
    // Read through the original database.
    while (read(fd, buffer, STUDENT_RECORD_SIZE) == STUDENT_RECORD_SIZE) {
        // Write only valid records.
        if (memcmp(buffer, zeroBuffer, STUDENT_RECORD_SIZE) != 0) {
            if (write(tmp_fd, buffer, STUDENT_RECORD_SIZE) != STUDENT_RECORD_SIZE) {
                printf(M_ERR_DB_WRITE);
                close(tmp_fd);
                return ERR_DB_FILE;
            }
        }
    }
    // Close the original and temporary files.
    close(fd);
    close(tmp_fd);
    // Replace the original database with the temporary file.
    if (rename(TMP_DB_FILE, DB_FILE) == -1) {
        printf(M_ERR_DB_CREATE);
        return ERR_DB_FILE;
    }
    // Reopen the compressed database file.
    int new_fd = open_db(DB_FILE, false);
    if (new_fd == ERR_DB_FILE) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }
    printf(M_DB_COMPRESSED_OK);
    return new_fd;
}

/*
 *  validate_range
 *      id:  Proposed student ID
 *      gpa: Proposed GPA
 *
 *  Validates that id and gpa are within the allowable ranges defined in db.h.
 *
 *  Returns:
 *      NO_ERROR       if valid,
 *      EXIT_FAIL_ARGS if either value is out of range.
 *
 *  Console:  No output.
 */
int validate_range(int id, int gpa) {
    if ((id < MIN_STD_ID) || (id > MAX_STD_ID))
        return EXIT_FAIL_ARGS;
    if ((gpa < MIN_STD_GPA) || (gpa > MAX_STD_GPA))
        return EXIT_FAIL_ARGS;
    return NO_ERROR;
}

/*
 *  usage
 *      exename:  Name of the executable (argv[0])
 *
 *  Prints the usage information.
 *
 *  Returns: Nothing.
 *
 *  Console:  Prints the usage.
 */
void usage(char *exename) {
    printf("usage: %s -[h|a|c|d|f|p|z] options.  Where:\n", exename);
    printf("\t-h:  prints help\n");
    printf("\t-a id first_name last_name gpa(as 3 digit int):  adds a student\n");
    printf("\t-c:  counts the records in the database\n");
    printf("\t-d id:  deletes a student\n");
    printf("\t-f id:  finds and prints a student in the database\n");
    printf("\t-p:  prints all records in the student database\n");
    printf("\t-x:  compress the database file [EXTRA CREDIT]\n");
    printf("\t-z:  zero db file (remove all records)\n");
}

// Main function.
int main(int argc, char *argv[]) {
    char opt;           // User-selected option
    int fd;             // File descriptor for the database
    int rc;             // Return code from operations
    int exit_code;      // Exit code to shell
    int id;             // Student ID (from argv)
    int gpa;            // GPA (from argv)

    student_t student = {0};  // Structure to hold student information

    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    // Get the option flag (e.g. -a, -c, -d, etc.)
    opt = *(argv[1] + 1);

    if (opt == 'h') {
        usage(argv[0]);
        exit(EXIT_OK);
    }

    fd = open_db(DB_FILE, false);
    if (fd < 0) {
        exit(EXIT_FAIL_DB);
    }

    exit_code = EXIT_OK;
    switch (opt) {
        case 'a':
            // Expected arguments: -a id first_name last_name gpa
            if (argc != 6) {
                usage(argv[0]);
                exit_code = EXIT_FAIL_ARGS;
                break;
            }
            id = atoi(argv[2]);
            gpa = atoi(argv[5]);
            exit_code = validate_range(id, gpa);
            if (exit_code == EXIT_FAIL_ARGS) {
                printf(M_ERR_STD_RNG);
                break;
            }
            rc = add_student(fd, id, argv[3], argv[4], gpa);
            if (rc < 0)
                exit_code = EXIT_FAIL_DB;
            break;
        case 'c':
            rc = count_db_records(fd);
            if (rc < 0)
                exit_code = EXIT_FAIL_DB;
            break;
        case 'd':
            if (argc != 3) {
                usage(argv[0]);
                exit_code = EXIT_FAIL_ARGS;
                break;
            }
            id = atoi(argv[2]);
            rc = del_student(fd, id);
            if (rc < 0)
                exit_code = EXIT_FAIL_DB;
            break;
        case 'f':
            if (argc != 3) {
                usage(argv[0]);
                exit_code = EXIT_FAIL_ARGS;
                break;
            }
            id = atoi(argv[2]);
            rc = get_student(fd, id, &student);
            switch (rc) {
                case NO_ERROR:
                    print_student(&student);
                    break;
                case SRCH_NOT_FOUND:
                    printf(M_STD_NOT_FND_MSG, id);
                    exit_code = EXIT_FAIL_DB;
                    break;
                default:
                    printf(M_ERR_DB_READ);
                    exit_code = EXIT_FAIL_DB;
                    break;
            }
            break;
        case 'p':
            rc = print_db(fd);
            if (rc < 0)
                exit_code = EXIT_FAIL_DB;
            break;
        case 'x':
            // Extra Credit: compress the database file.
            fd = compress_db(fd);
            if (fd < 0)
                exit_code = EXIT_FAIL_DB;
            break;
        case 'z':
            // Zero out the database file.
            close(fd);
            fd = open_db(DB_FILE, true);
            if (fd < 0) {
                exit_code = EXIT_FAIL_DB;
                break;
            }
            printf(M_DB_ZERO_OK);
            exit_code = EXIT_OK;
            break;
        default:
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
    }

    close(fd);
    exit(exit_code);
}