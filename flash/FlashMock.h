

class FlashMock  {

 public:
    FlashMock();
    ~FlashMock();

    void read(unsigned int address, void *buffer, unsigned int size);
    void write(unsigned int address, void *buffer, unsigned int size);
    void erase(unsigned int sector, unsigned int nun_sectors);

    int get_read_count();
    int get_write_count();
    int get_erase_count();

 private:
    int read_count = 0;
    int write_count = 0;
    int erase_count = 0;
};