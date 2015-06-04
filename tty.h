/*********************
 * tty raw connection
 *********************
 *
 */
class termios;

/*!
  \class Tty access class
  \brief Access tty in raw mode

  Open tty and set it to raw mode. Restore
  original mode on close
*/
class Tty
{
public:

    /*! Constructor
      \param max_terms maximal tty connection handles by the class
     */
    Tty(const int max_terms = DEF_MAXTERMS);

    /*! Destructor
      All open connections are closed
     */
    ~Tty();

    /*! Open tty connection

      Open tty connection and set it raw mode
      \param tty_port device to open, like /dev/tty0, /dev/ttyS0, etc.
      \param speed connection speed (like 9600, 115200, etc). If it zero
      the connection spee will not changed
      \param reopen if true reopen existing tty connection instead of
      opening the new one. In such case \e tty_fd have to be specified.
      This may be usable if tty connection is already opened in some other place
      and caller just wants to set raw mode
      \param tty_fd old terminal handler to reopen
      \return file descriptor of the tty connection on success or -1 on failure.
      errno is set in case of fauilure
    */
    int  open(const char *tty_port, const int speed = 0,
              const bool reopen = false, const int tty_fd = -1);

    /*! Close tty connection
      \param tid file descriptor of the tty connection to close
     */
    void close(const int tid);

private:
    static const int DEF_MAXTERMS;

    int      maxterms;
    termios  *defaults;
    int      *tty_h;
    void     setraw(termios& t, int speed);
    void     do_close(const int entry);
};
