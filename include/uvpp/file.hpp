#pragma once

#include "request.hpp"
#include "error.hpp"
#include "loop.hpp"

#include <memory>
#include <chrono>
#include <iostream>
#include <list>

#ifndef S_IRUSR
#   define S_IRUSR 00400
#endif

#ifndef S_IWUSR
#   define S_IWUSR 00200
#endif

namespace uvpp {

struct Stats
{
    int dev = 0;
    int mode = 0;
    int nlink = 0;
    int uid = 0;
    int gid = 0;
    int rdev = 0;
    double size = 0;
    double ino = 0;
    double atime = 0;
    double mtime = 0;
    double ctime = 0;
};

#define STAT_GET_DOUBLE(name) \
    static_cast<double>(s->st_##name)

#define STAT_GET_DATE(name) \
    (static_cast<double>(s->st_##name.tv_sec) * 1000) + \
    (static_cast<double>(s->st_##name.tv_nsec / 1000000))


inline Stats statsFromUV(const uv_stat_t *s)
{
    Stats stats;
    stats.dev = s->st_dev;
    stats.mode = s->st_mode;
    stats.nlink = s->st_nlink;
    stats.uid = s->st_uid;
    stats.gid = s->st_gid;
    stats.rdev = s->st_rdev;
    stats.size = STAT_GET_DOUBLE(size);
    stats.ino = STAT_GET_DOUBLE(ino);
    stats.atime = STAT_GET_DATE(atim);
    stats.mtime = STAT_GET_DATE(mtim);
    stats.ctime = STAT_GET_DATE(ctim);

    return stats;
}

struct ReadOptions
{
    int flags = O_CREAT | O_RDWR;
    int mode = S_IRUSR | S_IWUSR;
};

struct WriteOptions
{
    int flags = O_WRONLY | O_CREAT;
    int mode = S_IRUSR | S_IWUSR;
};

class File : public request<uv_fs_t>
{
public:

    class Entry
    {
        friend class File;
    public:
        const std::string &name()
        {
            return name_;
        }

        const std::string fullPath()
        {
            return fullPath_;
        }

        uv_dirent_type_t type()
        {
            return ent_.type;
        }

        File file()
        {
            return File(fullPath());
        }

        const std::string typeString()
        {
            switch (ent_.type)
            {
                case UV_DIRENT_FILE:
                    return "FILE";
                    break;
                case UV_DIRENT_DIR:
                    return "DIR";
                    break;
                case UV_DIRENT_LINK:
                    return "LINK";
                    break;
                case UV_DIRENT_FIFO:
                    return "FIFO";
                    break;
                case UV_DIRENT_SOCKET:
                    return "SOCKET";
                    break;
                case UV_DIRENT_CHAR:
                    return "CHAR";
                    break;
                case UV_DIRENT_BLOCK:
                    return "CHAR";
                    break;
                default:
                    return "UNKNONW";
            }
        }


    private:
        Entry(File *file, uv_dirent_t dir) : file_(file), ent_(dir)
        {
            name_ = std::string(ent_.name);
            if (ent_.type == UV_DIRENT_DIR)
                fullPath_ = file_->path_ + name_ + "/";
            else
                fullPath_ = file_->path_ + name_;
        };
        File *file_;
        uv_dirent_t ent_;
        std::string name_;
        std::string fullPath_;
    };

    File(const std::string &path) : request<uv_fs_t>(), loop_(uv_default_loop()), path_(path)
    {

    }

    File(Loop &l, const std::string &path) : request<uv_fs_t>(), loop_(l.get()), path_(path)
    {

    }

    Error open(int flags, int mode, CallbackWithResult callback)
    {

        auto openCallback = [this, callback](Error err, uv_file file)
        {
            if (!err)
                this->file_ = file;
            callback(err);
        };

        callbacks::store(get()->data, internal::uv_cid_fs_open, openCallback);

        return Error(uv_fs_open(loop_.get(), get(), path_.c_str(), flags, mode, [](uv_fs_t* req)
        {
            auto result = req->result;
            uv_fs_req_cleanup(req);

            if (result<0)
            {
                callbacks::invoke<decltype(openCallback)>(req->data, internal::uv_cid_fs_open, Error(result), result);
            }
            else
            {
                callbacks::invoke<decltype(openCallback)>(req->data, internal::uv_cid_fs_open, Error(0), result);
            }
        }));
    }

    Error read(int64_t bytes, int64_t offset, std::function<void(const char *buf, ssize_t len)> callback)
    {

        if (!file_) return Error(UV_EIO);

        uv_buf_t buffer;
        buffer.base = new char[bytes];
        buffer.len = bytes;

        auto readCallback = [this, callback, buffer](ssize_t result)
        {
            std::shared_ptr<char> baseHolder(buffer.base, std::default_delete<char[]>());
            
            if (!result)
            {
                callback(nullptr, result);
            }
            else
            {
                callback(buffer.base, result);
            }
        };

        callbacks::store(get()->data, internal::uv_cid_fs_read, readCallback);

        return Error(uv_fs_read(loop_.get(), get(), file_, &buffer, 1, offset, [](uv_fs_t* req)
        {
            auto result = req->result;
            uv_fs_req_cleanup(req);
            callbacks::invoke<decltype(readCallback)>(req->data, internal::uv_cid_fs_read, result);
        }));
    }

    Error write(const char* buf, int len, int offset, CallbackWithResult callback)
    {

        if (!file_) return Error(UV_EIO);

        callbacks::store(get()->data, internal::uv_cid_fs_write, callback);

        uv_buf_t bufs[] = { uv_buf_init(const_cast<char*>(buf), len) };

        return Error(uv_fs_write(loop_.get(), get(), file_, bufs, 1, offset, [](uv_fs_t* req)
        {
            auto result = req->result;
            uv_fs_req_cleanup(req);
            if (result < 0)
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_write, Error(result));
            }
            else
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_write, Error(0));
            }
        }));
    }

    Error close(std::function<void()> callback)
    {

        if (!file_) return Error(UV_EIO);

        callbacks::store(get()->data, internal::uv_cid_fs_close, callback);

        return Error(uv_fs_close(loop_.get(), get(), file_, [](uv_fs_t* req)
        {
            uv_fs_req_cleanup(req);
            callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_close);
        }));
    }

    Error close()
    {

        if (!file_) return Error(UV_EIO);

        return Error(uv_fs_close(loop_.get(), get(), file_, nullptr));
    }

    Error unlink(CallbackWithResult callback)
    {

        if (!file_) return Error(UV_EIO);

        callbacks::store(get()->data, internal::uv_cid_fs_unlink, callback);

        return Error(uv_fs_close(loop_.get(), get(), file_, [](uv_fs_t* req)
        {
            int result = req->result;
            uv_fs_req_cleanup(req);
            if (result < 0)
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_unlink, Error(result));
            }
            else
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_unlink, Error(0));
            }
        }));
    }

    Error unlink()
    {

        if (!file_) return Error(UV_EIO);

        return Error(uv_fs_close(loop_.get(), get(), file_, nullptr));
    }

    Error stats(std::function<void(Error err, Stats stats)> callback)
    {
        callbacks::store(get()->data, internal::uv_cid_fs_stats, callback);

        return Error(
                   uv_fs_stat(loop_.get(), get(), path_.c_str(), [](uv_fs_t* req)
        {
            int result = req->result;
            Stats stats;

            if (result >= 0)
            {

                auto s = static_cast<const uv_stat_t*>(req->ptr);

                stats = statsFromUV(s);
            }

            uv_fs_req_cleanup(req);

            if (result < 0)
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_stats, Error(result), stats);
            }
            else
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_stats, Error(0), stats);
            }

        })
               );
    }

    Stats stats()
    {
        int err = uv_fs_stat(loop_.get(), get(), path_.c_str(), nullptr);

        if (err>=0)
        {
            Stats stats;
            auto s = static_cast<const uv_stat_t*>(get()->ptr);
            stats = statsFromUV(s);
            uv_fs_req_cleanup(get());
            return stats;
        }
        else
        {
            uv_fs_req_cleanup(get());
            throw Exception(Error(err).str());
        }
    }

    Error fsync(CallbackWithResult callback)
    {

        if (!file_) return Error(UV_EIO);

        callbacks::store(get()->data, internal::uv_cid_fs_fsync, callback);

        return Error(
                   uv_fs_fsync(loop_.get(), get(), file_, [](uv_fs_t* req)
        {
            int result = req->result;

            uv_fs_req_cleanup(req);

            if (result < 0)
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_fsync, Error(result));
            }
            else
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_fsync, Error(0));
            }
        })
               );
    }

    Error rename(const std::string &newName, CallbackWithResult callback)
    {

        callbacks::store(get()->data, internal::uv_cid_fs_rename, callback);

        return Error(
                   uv_fs_rename(loop_.get(), get(), path_.c_str(), newName.c_str(), [](uv_fs_t* req)
        {
            int result = req->result;

            uv_fs_req_cleanup(req);

            if (result < 0)
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_rename, Error(result));
            }
            else
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_rename, Error(0));
            }
        })
               );
    }

    Error sendfile(const File &out, int64_t in_offset, size_t length, CallbackWithResult callback)
    {

        if (!file_) return Error(UV_EIO);
        if (!out.file_) return Error(UV_EIO);

        callbacks::store(get()->data, internal::uv_cid_fs_sendfile, callback);

        return Error(
                   uv_fs_sendfile(loop_.get(), get(), file_, out.file_, in_offset, length, [](uv_fs_t* req)
        {
            int result = req->result;

            uv_fs_req_cleanup(req);

            if (result < 0)
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_sendfile, Error(result));
            }
            else
            {
                callbacks::invoke<decltype(callback)>(req->data, internal::uv_cid_fs_sendfile, Error(0));
            }
        })
               );
    }

    Error scandir(std::function<void(Error err, std::list<Entry> files)> callback)
    {

        auto scanDirCallback = [this, callback](int result)
        {
            std::list<Entry> files;
            if (result < 0)
            {
                uv_fs_req_cleanup(this->get());
                callback(Error(result), files);
            }
            else
            {
                uv_dirent_t ent;
                for (int i=0; i<result; i++)
                {
                    Error err = Error(uv_fs_scandir_next(this->get(), &ent));
                    if (!err)
                    {
                        files.push_back(Entry(this,ent));
                    }
                }
                uv_fs_req_cleanup(this->get());
                callback(Error(0), files);
            }
        };

        callbacks::store(get()->data, internal::uv_cid_fs_scandir, scanDirCallback);

        return Error(
                   uv_fs_scandir(loop_.get(), get(), path_.c_str(), 0, [](uv_fs_t* req)
        {
            callbacks::invoke<decltype(scanDirCallback)>(req->data, internal::uv_cid_fs_scandir, req->result);
        })
               );
    }

    std::list<Entry> scandir()
    {
        int err = uv_fs_scandir(loop_.get(), get(), path_.c_str(), 0, nullptr);
        if (err >= 0)
        {
            std::list<Entry> files;
            uv_dirent_t ent;
            int r;
            while ( (r = uv_fs_scandir_next(get(), &ent))!= UV_EOF )
            {
                files.push_back(Entry(this, ent));
            }
            uv_fs_req_cleanup(get());
            return files;
        }
        else
        {
            throw Exception(Error(err).str());
        }
    }



private:
    const std::string path_;
    Loop loop_;
    uv_file file_=0;
};


}