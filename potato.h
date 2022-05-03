class Potato {
    private:
        int hops_num;
        int num_pass;
        int trace[512];
    public:
        Potato() : hops_num(0), num_pass(0) {
            memset(trace, 0, sizeof(trace));
        }
        void print_trace() {
            for (int i = 0; i < num_pass - 1; i++) {
                printf("%d,", trace[i]);
            }
            printf("%d\n", trace[num_pass - 1]);
        }
        void set_hop_num(int hop_num) {
            hops_num = hop_num;
        }
        int get_hops_left() {
            return hops_num;
        }
        void hops_minus_one() {
            hops_num = hops_num - 1;
        }
        void num_pass_plus_one() {
            num_pass = num_pass + 1;
        }
        void trace_add_this(int cur_player_no) {
            trace[num_pass - 1] = cur_player_no;
        }
};