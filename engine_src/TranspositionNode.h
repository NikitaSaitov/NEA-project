using U64 = unsigned long long;

extern "C" {

    struct TranspositionNode{

        U64 hashKey = 0;
        int depth = 0;
        int flag = 0;
        int score = 0;

    };
}
