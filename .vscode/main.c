typedef unsigned long long Bitboard;

/* Player structure: men and kings */
typedef struct {
    Bitboard men;
    Bitboard kings;
} Player;

/* Game structure */
typedef struct {
    Player red;
    Player black;
    int turn; // 0 = Red, 1 = Black
} Game;

/* --- Bitboard helpers --- */
Bitboard get_bit(int row, int col) {
    return 1ULL << (row * 8 + col);
}

void set_bit(Bitboard *board, int row, int col) {
    *board |= get_bit(row, col);
}

void clear_bit(Bitboard *board, int row, int col) {
    *board &= ~get_bit(row, col);
}

int is_bit_set(Bitboard board, int row, int col) {
    return (board & get_bit(row, col)) != 0;
}

/* --- Initialize game --- */
void init_game(Game *game) {
    game->red.men = game->red.kings = 0ULL;
    game->black.men = game->black.kings = 0ULL;
    game->turn = 0;

    /* Black pieces on top 3 rows */
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 8; c++)
            if ((r + c) % 2) set_bit(&game->black.men, r, c);

    /* Red pieces on bottom 3 rows */
    for (int r = 5; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if ((r + c) % 2) set_bit(&game->red.men, r, c);
}

/* --- Display board --- */
void print_board(Game *game) {
    printf("\n  0 1 2 3 4 5 6 7\n  ----------------\n");
    for (int r = 0; r < 8; r++) {
        printf("%d| ", r);
        for (int c = 0; c < 8; c++) {
            char piece = '.';
            if (is_bit_set(game->red.men, r, c)) piece = 'r';
            else if (is_bit_set(game->red.kings, r, c)) piece = 'R';
            else if (is_bit_set(game->black.men, r, c)) piece = 'b';
            else if (is_bit_set(game->black.kings, r, c)) piece = 'B';
            printf("%c ", piece);
        }
        printf("\n");
    }
    printf("Turn: %s\n", game->turn == 0 ? "Red" : "Black");
}

/* --- Crown king if reaching last row --- */
void crown_king(Game *game) {
    for (int c = 0; c < 8; c++) {
        if (is_bit_set(game->red.men, 0, c)) {
            clear_bit(&game->red.men, 0, c);
            set_bit(&game->red.kings, 0, c);
        }
        if (is_bit_set(game->black.men, 7, c)) {
            clear_bit(&game->black.men, 7, c);
            set_bit(&game->black.kings, 7, c);
        }
    }
}

/* --- Check if a square is occupied --- */
int is_occupied(Game *game, int row, int col) {
    return is_bit_set(game->red.men | game->red.kings | game->black.men | game->black.kings, row, col);
}

/* --- Validate move --- */
int valid_move(Game *game, int r1, int c1, int r2, int c2) {
    if (r2 < 0 || r2 > 7 || c2 < 0 || c2 > 7) return 0;
    if (is_occupied(game, r2, c2)) return 0;

    Player *player = game->turn ? &game->black : &game->red;
    Player *enemy = game->turn ? &game->red : &game->black;

    int dr = r2 - r1;
    int dc = c2 - c1;
    int dir = game->turn ? 1 : -1; // Red moves up (-1), Black moves down (+1)

    // King can move both directions
    if (is_bit_set(player->kings, r1, c1)) dir = dr;

    // Normal move
    if (abs(dr) == 1 && abs(dc) == 1 && dr == dir) return 1;

    // Jump move
    if (abs(dr) == 2 && abs(dc) == 2) {
        int mr = (r1 + r2)/2;
        int mc = (c1 + c2)/2;
        if (is_bit_set(enemy->men | enemy->kings, mr, mc)) return 2;
    }

    return 0;
}

/* --- Move piece --- */
int move_piece(Game *game, int r1, int c1, int r2, int c2) {
    int move_type = valid_move(game, r1, c1, r2, c2);
    if (!move_type) return 0;

    Player *player = game->turn ? &game->black : &game->red;
    Player *enemy = game->turn ? &game->red : &game->black;
    int king = is_bit_set(player->kings, r1, c1);

    // Remove from old square
    clear_bit(&player->men, r1, c1);
    clear_bit(&player->kings, r1, c1);

    // Place on new square
    if (king) set_bit(&player->kings, r2, c2);
    else set_bit(&player->men, r2, c2);

    // Remove captured piece if jump
    if (move_type == 2) {
        int mr = (r1 + r2)/2;
        int mc = (c1 + c2)/2;
        clear_bit(&enemy->men, mr, mc);
        clear_bit(&enemy->kings, mr, mc);
    }

    crown_king(game);
    game->turn = !game->turn; // switch turn
    return 1;
}

/* --- Check if player has pieces --- */
int has_pieces(Player *p) {
    return (p->men | p->kings) != 0;
}

/* --- Main game loop --- */
int main() {
    Game game;
    init_game(&game);

    int r1, c1, r2, c2;

    while (1) {
        print_board(&game);

        if (!has_pieces(&game.red)) { printf("\nBlack wins!\n"); break; }
        if (!has_pieces(&game.black)) { printf("\nRed wins!\n"); break; }

        printf("Enter move (r1 c1 r2 c2): ");
        if (scanf("%d %d %d %d", &r1, &c1, &r2, &c2) != 4) break;

        if (!move_piece(&game, r1, c1, r2, c2)) {
            printf("Invalid move! Try again.\n");
        }
    }

    return 0;
}