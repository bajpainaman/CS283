## Assignment 2 Questions

#### Directions
Please answer the following questions and submit in your repo for the second assignment.  Please keep the answers as short and concise as possible.

1. In this assignment I asked you provide an implementation for the `get_student(...)` function because I think it improves the overall design of the database application.   After you implemented your solution do you agree that externalizing `get_student(...)` into it's own function is a good design strategy?  Briefly describe why or why not.

    > **Answer**:  
Externalizing the `get_student(...)` function is a smart move. By isolating the logic that fetches a student’s data, the code becomes more modular and easier to manage. It cuts down on clutter and repetition, so if you need to tweak how a student is retrieved, you only have to change it in one place instead of sifting through a tangled mess of code. In short, this design strategy keeps things clean, testable, and maintainable, which is pretty much the goal when you’re building a scalable system.



2. Another interesting aspect of the `get_student(...)` function is how its function prototype requires the caller to provide the storage for the `student_t` structure:

    ```c
    int get_student(int fd, int id, student_t *s);
    ```

    Notice that the last parameter is a pointer to storage **provided by the caller** to be used by this function to populate information about the desired student that is queried from the database file. This is a common convention (called pass-by-reference) in the `C` programming language. 

    In other programming languages an approach like the one shown below would be more idiomatic for creating a function like `get_student()` (specifically the storage is provided by the `get_student(...)` function itself):

    ```c
    //Lookup student from the database
    // IF FOUND: return pointer to student data
    // IF NOT FOUND: return NULL
    student_t *get_student(int fd, int id){
        student_t student;
        bool student_found = false;
        
        //code that looks for the student and if
        //found populates the student structure
        //The found_student variable will be set
        //to true if the student is in the database
        //or false otherwise.

        if (student_found)
            return &student;
        else
            return NULL;
    }
    ```
    Can you think of any reason why the above implementation would be a **very bad idea** using the C programming language?  Specifically, address why the above code introduces a subtle bug that could be hard to identify at runtime? 

    > **ANSWER:** >
Returning a pointer to a local variable in the alternative implementation is a total disaster waiting to happen. When you declare a variable (like `student_t student;`) on the stack and then return its address, that variable ceases to exist once the function finishes executing. This leaves you with a dangling pointer—a sneaky bug that might work sometimes and crash at the worst moment, making it super hard to track down. It’s like trying to hang on to a ghost: it’s not really there, and anything relying on it is bound to fail.

3. Another way the `get_student(...)` function could be implemented is as follows:

    ```c
    //Lookup student from the database
    // IF FOUND: return pointer to student data
    // IF NOT FOUND or memory allocation error: return NULL
    student_t *get_student(int fd, int id){
        student_t *pstudent;
        bool student_found = false;

        pstudent = malloc(sizeof(student_t));
        if (pstudent == NULL)
            return NULL;
        
        //code that looks for the student and if
        //found populates the student structure
        //The found_student variable will be set
        //to true if the student is in the database
        //or false otherwise.

        if (student_found){
            return pstudent;
        }
        else {
            free(pstudent);
            return NULL;
        }
    }
    ```
    In this implementation the storage for the student record is allocated on the heap using `malloc()` and passed back to the caller when the function returns. What do you think about this alternative implementation of `get_student(...)`?  Address in your answer why it work work, but also think about any potential problems it could cause.  
    
    > **ANSWER:** 
> **ANSWER 3:**  
Using `malloc()` to allocate memory for the student record on the heap solves the dangling pointer problem since the memory sticks around after the function returns. However, it comes with its own baggage. The caller now has to remember to free the memory to avoid leaks, which can lead to messy, inefficient code if not handled properly. Plus, heap allocation is generally slower and can fail if the system runs out of memory. So while this approach works and is sometimes necessary, it introduces extra responsibility and potential pitfalls that you need to be on guard for.


4. Lets take a look at how storage is managed for our simple database. Recall that all student records are stored on disk using the layout of the `student_t` structure (which has a size of 64 bytes).  Lets start with a fresh database by deleting the `student.db` file using the command `rm ./student.db`.  Now that we have an empty database lets add a few students and see what is happening under the covers.  Consider the following sequence of commands:

    ```bash
    > ./sdbsc -a 1 john doe 345
    > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 128 Jan 17 10:01 ./student.db
    > du -h ./student.db
        4.0K    ./student.db
    > ./sdbsc -a 3 jane doe 390
    > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 256 Jan 17 10:02 ./student.db
    > du -h ./student.db
        4.0K    ./student.db
    > ./sdbsc -a 63 jim doe 285 
    > du -h ./student.db
        4.0K    ./student.db
    > ./sdbsc -a 64 janet doe 310
    > du -h ./student.db
        8.0K    ./student.db
    > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 4160 Jan 17 10:03 ./student.db
    ```

    For this question I am asking you to perform some online research to investigate why there is a difference between the size of the file reported by the `ls` command and the actual storage used on the disk reported by the `du` command.  Understanding why this happens by design is important since all good systems programmers need to understand things like how linux creates sparse files, and how linux physically stores data on disk using fixed block sizes.  Some good google searches to get you started: _"lseek syscall holes and sparse files"_, and _"linux file system blocks"_.  After you do some research please answer the following:

    - Please explain why the file size reported by the `ls` command was 128 bytes after adding student with ID=1, 256 after adding student with ID=3, and 4160 after adding the student with ID=64? 

        > **ANSWER:** >  
The `ls` command shows the logical size of the file, which reflects the highest byte offset that’s been written plus the size of the last record. Since each student record is 64 bytes, adding a student with ID=1 bumps the file size to 128 bytes, ID=3 pushes it to 256 bytes, and ID=64 forces it to expand to 4160 bytes. The file size increases because the program is writing the student data at an offset based on the student’s ID, so even if there are gaps in between, the size reported by `ls` jumps to cover the highest offset plus one full record.



    -   Why did the total storage used on the disk remain unchanged when we added the student with ID=1, ID=3, and ID=63, but increased from 4K to 8K when we added the student with ID=64? 

        > **ANSWER:** > 
Even though the logical file size increases with each new record, the actual disk usage (as reported by `du`) doesn’t change until you cross a filesystem block boundary. Linux filesystems allocate space in fixed blocks—often 4K in size. When you add students with IDs 1, 3, and 63, you’re mostly creating gaps (or “holes”) that aren’t actually stored on disk because of sparse file support. It’s only when you add the student with ID=64 that the file’s data finally spills over into a new 4K block, causing the actual allocated disk space to jump from 4K to 8K.

 

    - Now lets add one more student with a large student ID number  and see what happens:

        ```bash
        > ./sdbsc -a 99999 big dude 205 
        > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 6400000 Jan 17 10:28 ./student.db
        > du -h ./student.db
        12K     ./student.db
        ```
        We see from above adding a student with a very large student ID (ID=99999) increased the file size to 6400000 as shown by `ls` but the raw storage only increased to 12K as reported by `du`.  Can provide some insight into why this happened?

        > **ANSWER:** 
When you add a student with a huge ID like 99999, the logical file size balloons (to 6,400,000 bytes in this case) because the record is placed at a very high offset. However, thanks to sparse file allocation, the filesystem doesn’t actually reserve disk space for all those unwritten gaps. It only allocates blocks for the parts of the file that have real data, which is why the actual disk usage only goes up to 12K. In essence, you’re creating a massive “hole” in the file that looks huge logically but takes up almost no physical space—an ingenious trick by modern filesystems to save storage.
