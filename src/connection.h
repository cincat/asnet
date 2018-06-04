
namespace asnet {
class Stream;
class Connection {
public:
    Connection(Stream *local, Stream *remote): local_(local), remote_(remote) {
    }

    Stream* getLocal() {return local_;}
    Stream* getRemote() {return remote_;}
private:
    Stream *local_, *remote_;
};
}//end of namespace asnet