#include "gitbridge.h"
#include <QDebug>
#include <QSettings>
#include <stdio.h>

GitBridge::GitBridge() {
    loadRepository();
}

GitBridge::~GitBridge() {
    freeResources();
}

void GitBridge::errorManager(const int &error, const char *desc) {
    if (error < 0) {
        const git_error *e = git_error_last();
        printf("Error %d/%d: %s    %s\n", error, e->klass, e->message, desc);
        //exit(error);
    }
}

void GitBridge::freeResources() {
    //if(repo)
    //    git_repository_free(repo);

    // unload from memory, also initiates clean-up
    git_libgit2_shutdown();
}

void GitBridge::loadRepository() {
    // load library into memory
    git_libgit2_init();
    QSettings settings("Evolution-IDE");
    const QString path = settings.value("Evolution-IDE/git_paht").toString();
    if(path.isEmpty()){
        qDebug() << "empty repo path !";
        return;
    }
    errorManager(git_repository_open(&repo, path.toLatin1().data()), "cannot load repository.");
}

int GitBridge::commit(const char *comment) {
    int error;

    git_oid commit_oid, tree_oid;
    git_tree *tree;
    git_index *index;
    git_object *parent = NULL;
    git_reference *ref = NULL;
    git_signature *signature;

    errorManager(git_revparse_ext(&parent, &ref, repo, "HEAD"), "cannot find object.");

    errorManager(git_repository_index(&index, repo), "cannot get index for repository.");
    errorManager(git_index_write_tree(&tree_oid, index), "cannot write tree index.");
    errorManager(git_index_write(index), "cannot write index.");

    errorManager(git_tree_lookup(&tree, repo, &tree_oid), "cannot lookup tree");
    errorManager(git_signature_default(&signature, repo), "cannot get signature.");

    errorManager(git_commit_create_v(
            &commit_oid,
            repo,
            "HEAD",
            signature,
            signature,
            NULL,
            comment,
            tree,
            parent ? 1 : 0, parent), "cannot create commit.");

    git_index_free(index);
    git_signature_free(signature);
    git_tree_free(tree);

    return error;
}


void GitBridge::describe(describe_options *opts) {
    if (opts->commit_count == 0) {
        //do_describe_single(repo, opts, NULL);
    } else {
        size_t i;
        for (i = 0; i < opts->commit_count; i++) {
            //do_describe_single(repo, opts, opts->commits[i]);
        }
    }
}

int GitBridge::push(char *refspec) {
    git_push_options options;
    git_remote *remote = NULL;
    const git_strarray refspecs = {
            &refspec,
            1};

    errorManager(git_remote_lookup(&remote, repo, "origin"), "cannot get info about remote.");
    errorManager(git_push_options_init(&options, GIT_PUSH_OPTIONS_VERSION), "cannot init push option structure.");
    errorManager(git_remote_push(remote, &refspecs, &options), "cannot preform a push.");

    printf("push exited successfully");
    return 0;
}

int GitBridge::init(const char *RepoName, const bool &initCommit) {
    initOpts Opts = {1, 0, 0, 0, GIT_REPOSITORY_INIT_SHARED_UMASK, 0, 0, 0};
    Opts.shared = GIT_REPOSITORY_INIT_SHARED_ALL;

    git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
    /* Customize options */
    opts.flags |= GIT_REPOSITORY_INIT_MKPATH; /* mkdir as needed to create repo */
    opts.description = "My repository has a custom description";

    errorManager(git_repository_init_ext(&repo, RepoName, &opts), "cannot initialize repository.");

    /* Initialize repository. */

    if (Opts.no_options) {
        /*
        if true, a Git repository without a working directory is created at the pointed path.
        If false, provided path will be considered as the working directory into which the .git directory will be created.
        */
        errorManager(git_repository_init(&repo, RepoName, false), "cannot initialize repository.");
    }
    else {
        /**
         * Some command line options were specified, so we'll use the
         * extended init API to handle them
         */
        git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
        opts.flags = GIT_REPOSITORY_INIT_MKPATH;

        if (Opts.bare)
            opts.flags |= GIT_REPOSITORY_INIT_BARE;

        if (Opts.Template) {
            opts.flags |= GIT_REPOSITORY_INIT_EXTERNAL_TEMPLATE;
            opts.template_path = Opts.Template;
        }

        if (Opts.gitdir) {
            /**
             * If you specified a separate git directory, then initialize
             * the repository at that path and use the second path as the
             * working directory of the repository (with a git-link file)
             */
            opts.workdir_path = Opts.dir;
            Opts.dir = Opts.gitdir;
        }

        if (Opts.shared != 0)
            opts.mode = Opts.shared;

        errorManager(git_repository_init_ext(&repo, RepoName, &opts), "cannot initialize repository.");
    }

    /** Print a message to stdout like "git init" does. */

    if (!Opts.quiet) {
        if (Opts.bare || Opts.gitdir)
            Opts.dir = git_repository_path(repo);
        else
            Opts.dir = git_repository_workdir(repo);

        printf("Initialized empty Git repository in %s\n", Opts.dir);
    }

    /**
     * As an extension to the basic "git init" command, this example
     * gives the option to create an empty initial commit.  This is
     * mostly to demonstrate what it takes to do that, but also some
     * people like to have that empty base commit in their repo.
     */
    if (initCommit) {
        initialCommit();
        printf("Created empty initial commit\n");
    }

    git_repository_free(repo);

    return 0;
}

void GitBridge::initialCommit() {
    git_signature *sig; // user.name, user.email
    git_index *index;
    git_oid tree_id, commit_id;
    git_tree *tree;

    /** First use the config to initialize a commit signature for the user. */

    errorManager(git_signature_default(&sig, repo), "cannot create a commit signature.");

    /* Now let's create an empty tree for this commit */

    errorManager(git_repository_index(&index, repo), "cannot open repository index.");

    /**
     * Outside of this example, you could call git_index_add_bypath()
     * here to put actual files into the index.  For our purposes, we'll
     * leave it empty for now.
     */

    errorManager(git_index_write_tree(&tree_id, index), "cannot write tree index.");

    git_index_free(index);

    if (git_tree_lookup(&tree, repo, &tree_id) < 0)
        printf("Could not look up initial tree");

    /**
     * Ready to create the initial commit.
     *
     * Normally creating a commit would involve looking up the current
     * HEAD commit and making that be the parent of the initial commit,
     * but here this is the first commit so there will be no parent.
     */

    errorManager(git_commit_create_v(
            &commit_id, repo, "HEAD", sig, sig,
            NULL, "Initial commit", tree, 0), "cannot create initial commit.");

    /** Clean up so we don't leak memory. */

    git_tree_free(tree);
    git_signature_free(sig);
}

int GitBridge::add(const char *file) {
    git_index_matched_path_cb matched_cb = NULL;
    git_index *index;
    git_strarray array{(char **)file, 1};
    index_options options;

    /* Grab the repository's index. */
    errorManager(git_repository_index(&index, repo), "Could not open repository index");

    options.repo = repo;

    /* Perform the requested action with the index and files */
    if (options.add_update) {
        errorManager(git_index_update_all(index, &array, matched_cb, &options), "cannot update indices.");
    } else {
        errorManager(git_index_add_all(index, &array, 0, matched_cb, &options), "cannot index all.");
    }

    errorManager(git_index_add_bypath(index, file), "cannot add file.");

    /* Cleanup memory */
    git_index_write(index);
    git_index_free(index);

    return 0;
}

int GitBridge::remove(const char *file) {
    git_strarray array = {(char **)file, 1};
    git_index_matched_path_cb matched_cb = NULL;
    git_index *index;
    index_options options;

    errorManager(git_repository_index(&index, repo), "Could not open repository index");

    /* Perform the requested action with the index and files */
    if (options.add_update) {
        errorManager(git_index_update_all(index, &array, matched_cb, &options), "cannot update indices.");
    } else {
        errorManager(git_index_add_all(index, &array, 0, matched_cb, &options), "cannot index all.");
    }

    errorManager(git_index_remove_bypath(index, file), "cannot remove file.");
}

int GitBridge::clone(const char *dest, const char *url) {
    progress_data pd;
    git_repository *cloned_repo = NULL;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    /* Set up options */
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    clone_opts.checkout_opts = checkout_opts;
    //checkout_opts.progress_cb = checkout_progress;
    //checkout_opts.progress_payload = &pd;
    //clone_opts.fetch_opts.callbacks.sideband_progress = sideband_progress;
    //clone_opts.fetch_opts.callbacks.transfer_progress = &fetch_progress;
    //clone_opts.fetch_opts.callbacks.credentials = cred_acquire_cb;
    //clone_opts.fetch_opts.callbacks.payload = &pd;

    /* Do the clone */
    errorManager(git_clone(&cloned_repo, url, dest, &clone_opts), "cannot clone repository.");
    if (cloned_repo)
        git_repository_free(cloned_repo);

    return 0;
}

int GitBridge::fetch(const char *repo_name) {
    git_remote *remote = NULL;
    const git_indexer_progress *stats;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

    /* Figure out whether it's a named remote or a URL */
    printf("Fetching for repo \n");
    if (git_remote_lookup(&remote, repo, repo_name) < 0)
        if (git_remote_create_anonymous(&remote, repo, repo_name) < 0)
            goto on_error;

    /* Set up the callbacks (only update_tips for now) */
    //fetch_opts.callbacks.update_tips = &update_cb;
    //fetch_opts.callbacks.sideband_progress = &progress_cb;
    //fetch_opts.callbacks.transfer_progress = transfer_progress_cb;
    //fetch_opts.callbacks.credentials = cred_acquire_cb;

    /**
     * Perform the fetch with the configured refspecs from the
     * config. Update the reflog for the updated references with
     * "fetch".
     */
    if (git_remote_fetch(remote, NULL, &fetch_opts, "fetch") < 0)
        goto on_error;

    /**
     * If there are local objects (we got a thin pack), then tell
     * the user how many objects we saved from having to cross the
     * network.
     */
    stats = git_remote_stats(remote);
    if (stats->local_objects > 0) {
        printf("\rReceived %u/%u objects in % bytes (used %u local objects)\n",
               stats->indexed_objects, stats->total_objects, stats->received_bytes, stats->local_objects);
    } else{
        printf("\rReceived %u/%u objects in % bytes\n",
            stats->indexed_objects, stats->total_objects, stats->received_bytes);
    }

    git_remote_free(remote);

    return 0;

    on_error:
    git_remote_free(remote);
    return -1;
}


const char *GitBridge::findRepo(const char *repo_name) {
    char *buffer = nullptr;
    git_buf root = {buffer, 0, 0};
    errorManager(git_repository_discover(&root, repo_name, 0, NULL), "cannot find Repository");
    const char *path = root.ptr;
    git_buf_free(&root); // returned path data must be freed after use
    return path;
}

int GitBridge::status() {
    git_status_list *status;
    git_status_options o = GIT_STATUS_OPTIONS_INIT;

    o.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    o.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
              GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
              GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

    errorManager(git_repository_is_bare(repo), "cannot get info if repostitory is bare or not,");

    errorManager(git_status_list_new(&status, repo, &o), "cannot gather status list.");

    git_status_list_free(status);
    return 0;
}

const char *GitBridge::getBranch() {
    const char *branch = NULL;
    git_reference *head = NULL;

    errorManager(git_repository_head(&head, repo), "failed to get current branch");
    branch = git_reference_shorthand(head);
    git_reference_free(head);

    return branch;
}

/**
 * This function print out an output similar to git's status command
 * in long form, including the command-line hints.
 */
void GitBridge::getStatusData(git_status_list *status) {
    size_t i, maxi = git_status_list_entrycount(status);
    const git_status_entry *s;
    int header = 0, changes_in_index = 0;
    int changed_in_workdir = 0, rm_in_workdir = 0;
    const char *old_path, *new_path;

    /** Print index changes. */

    for (i = 0; i < maxi; ++i) {
        char *istatus = NULL;

        s = git_status_byindex(status, i);

        if (s->status == GIT_STATUS_CURRENT)
            continue;

        if (s->status & GIT_STATUS_WT_DELETED)
            rm_in_workdir = 1;

        if (s->status & GIT_STATUS_INDEX_NEW)
            istatus = "new file: ";
        if (s->status & GIT_STATUS_INDEX_MODIFIED)
            istatus = "modified: ";
        if (s->status & GIT_STATUS_INDEX_DELETED)
            istatus = "deleted:  ";
        if (s->status & GIT_STATUS_INDEX_RENAMED)
            istatus = "renamed:  ";
        if (s->status & GIT_STATUS_INDEX_TYPECHANGE)
            istatus = "typechange:";

        if (istatus == NULL)
            continue;

        if (!header) {
            printf("# Changes to be committed:\n");
            printf("#   (use \"git reset HEAD <file>...\" to unstage)\n");
            printf("#\n");
            header = 1;
        }

        old_path = s->head_to_index->old_file.path;
        new_path = s->head_to_index->new_file.path;

        if (old_path && new_path && strcmp(old_path, new_path))
            printf("#\t%s  %s -> %s\n", istatus, old_path, new_path);
        else
            printf("#\t%s  %s\n", istatus, old_path ? old_path : new_path);
    }

    if (header) {
        changes_in_index = 1;
        printf("#\n");
    }
    header = 0;

    /** Print workdir changes to tracked files. */

    for (i = 0; i < maxi; ++i) {
        char *wstatus = NULL;

        s = git_status_byindex(status, i);

        /**
         * With `GIT_STATUS_OPT_INCLUDE_UNMODIFIED` (not used in this example)
         * `index_to_workdir` may not be `NULL` even if there are
         * no differences, in which case it will be a `GIT_DELTA_UNMODIFIED`.
         */
        if (s->status == GIT_STATUS_CURRENT || s->index_to_workdir == NULL)
            continue;

        /** Print out the output since we know the file has some changes */
        if (s->status & GIT_STATUS_WT_MODIFIED)
            wstatus = "modified: ";
        if (s->status & GIT_STATUS_WT_DELETED)
            wstatus = "deleted:  ";
        if (s->status & GIT_STATUS_WT_RENAMED)
            wstatus = "renamed:  ";
        if (s->status & GIT_STATUS_WT_TYPECHANGE)
            wstatus = "typechange:";

        if (wstatus == NULL)
            continue;

        if (!header) {
            printf("# Changes not staged for commit:\n");
            printf("#   (use \"git add%s <file>...\" to update what will be committed)\n", rm_in_workdir ? "/rm" : "");
            printf("#   (use \"git checkout -- <file>...\" to discard changes in working directory)\n");
            printf("#\n");
            header = 1;
        }

        old_path = s->index_to_workdir->old_file.path;
        new_path = s->index_to_workdir->new_file.path;

        if (old_path && new_path && strcmp(old_path, new_path))
            printf("#\t%s  %s -> %s\n", wstatus, old_path, new_path);
        else
            printf("#\t%s  %s\n", wstatus, old_path ? old_path : new_path);
    }

    if (header) {
        changed_in_workdir = 1;
        printf("#\n");
    }

    /** Print untracked files. */

    header = 0;

    for (i = 0; i < maxi; ++i) {
        s = git_status_byindex(status, i);

        if (s->status == GIT_STATUS_WT_NEW) {

            if (!header) {
                printf("# Untracked files:\n");
                printf("#   (use \"git add <file>...\" to include in what will be committed)\n");
                printf("#\n");
                header = 1;
            }

            printf("#\t%s\n", s->index_to_workdir->old_file.path);
        }
    }

    header = 0;

    /** Print ignored files. */

    for (i = 0; i < maxi; ++i) {
        s = git_status_byindex(status, i);

        if (s->status == GIT_STATUS_IGNORED) {

            if (!header) {
                printf("# Ignored files:\n");
                printf("#   (use \"git add -f <file>...\" to include in what will be committed)\n");
                printf("#\n");
                header = 1;
            }

            printf("#\t%s\n", s->index_to_workdir->old_file.path);
        }
    }

    if (!changes_in_index && changed_in_workdir)
        printf("no changes added to commit (use \"git add\" and/or \"git commit -a\")\n");
}

