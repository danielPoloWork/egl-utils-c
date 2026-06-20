# Software Specification: d4np-c (C Systems Utility Library)

## 1. Descrizione & Filosofia di Design
`d4np-c` è una libreria statica/dinamica scritta in ANSI C (C99) pensata per fornire strutture dati e utility di sistema ad alte prestazioni. 
La filosofia di design si basa su:
*   **Gestione Esplicita della Memoria:** Tutte le strutture dati accettano un allocatore personalizzato (`d4np_allocator_t`) per evitare la dipendenza diretta da `malloc`/`free`.
*   **Error-handling deterministico:** Ritorno costante dei codici di stato (`d4np_status_t`) con strutture per catturare stack trace o messaggi d'errore nel contesto del thread.
*   **Zero cost abstractions:** Strutture dati allineate alla cache e ottimizzate per minimizzare i cache miss.

---

## 2. Specifiche delle Funzionalità (25 Punti da Sviluppare)

### Gestione Memoria & Allocatori
1.  **`d4np_arena_init`:** Inizializza un allocatore di tipo Arena per allocazioni veloci e deallocazione in blocco.
2.  **`d4np_arena_alloc`:** Alloca memoria dall'Arena a tempo $O(1)$.
3.  **`d4np_arena_reset`:** Libera tutta la memoria dell'arena senza rilasciare il blocco principale.
4.  **`d4np_pool_init`:** Crea un allocatore a blocchi di dimensione fissa per prevenire frammentazione.
5.  **`d4np_pool_alloc` / `d4np_pool_free`:** Allocazione e rilascio a blocchi costanti.

### Strutture Dati Generiche
6.  **`d4np_vector_t`:** Array dinamico riutilizzabile con ridimensionamento geometrico.
7.  **`d4np_hashmap_t`:** Tabella hash basata su open addressing con linear probing per cache friendliness.
8.  **`d4np_linked_list_t`:** Lista doppiamente concatenata con puntatori intrusivi.
9.  **`d4np_ring_buffer_t`:** Buffer circolare thread-safe per code produttore-consumatore.
10. **`d4np_string_builder_t`:** Costruttore di stringhe dinamico ed efficiente.

### Concorrenza & Sincronizzazione
11. **`d4np_mutex_t`:** Astrazione portabile sopra pthread/win32 mutex.
12. **`d4np_thread_pool_t`:** Pool di thread nativi con coda dei task interna.
13. **`d4np_atomic_queue_t`:** Coda lock-free single-producer single-consumer (SPSC) usando operazioni atomiche.
14. **`d4np_semaphore_t`:** Semaforo di sistema per la sincronizzazione inter-processo.

### Stringhe & Parsing
15. **`d4np_str_view_t`:** Astrazione non allocante sopra fette di stringhe (`const char*` + length).
16. **`d4np_str_split`:** Divisione efficiente di stringhe senza allocazione di memoria extra.
17. **`d4np_str_parse_int` / `d4np_str_parse_float`:** Conversione stringa-numero robusta e sicura contro l'overflow.

### File System & I/O
18. **`d4np_file_read_all`:** Legge un intero file in un buffer dinamico gestendo gli errori di lettura.
19. **`d4np_file_write_all`:** Scrive un intero buffer su disco garantendo il flush atomico.
20. **`d4np_path_combine`:** Unisce in modo sicuro percorsi del file system rispettando il separatore del SO.

### Utilità di Sistema & Diagnostica
21. **`d4np_log_write`:** Logger configurabile con livelli (INFO, WARN, ERROR) e output su console o file.
22. **`d4np_error_context_push` / `d4np_error_context_pop`:** Tracciamento dell'errore corrente per thread.
23. **`d4np_timestamp_ms`:** Restituisce il timestamp corrente a precisione di millisecondi (monotonico).
24. **`d4np_uuid_generate`:** Generazione veloce di UUID v4 conformi allo standard RFC4122.
25. **`d4np_hash_fnv1a`:** Implementazione dell'algoritmo di hashing FNV-1a per stringhe e buffer binari.

---

## 3. Esempio API (Uso della String View)

```c
#include "d4np_c.h"
#include <stdio.h>

int main() {
    d4np_str_view_t sv = d4np_str_view_from_str("Daniel;Polo;Architect");
    d4np_str_view_t token;
    
    // Split senza allocare memoria
    while (d4np_str_view_split_next(&sv, ';', &token)) {
        printf("Token: %.*s\n", (int)token.len, token.ptr);
    }
    return 0;
}
```
