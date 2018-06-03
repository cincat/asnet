
namespace asnet {
class Stream;
class Event {
public:
    enum Type{
        DATA,
        ACCEPT,
        CONNECT,
        TIMEOUT,
        TICTOK
    };
    Type type;
    Stream *local_, *remote_;

    Event(Stream *local, Stream *remote): local_(local), remote_(remote) {
    }

    Stream* getLocal() {return local_;}
    Stream* getRemote() {return remote_;}
};
}//end of namespace asnet