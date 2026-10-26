# Putting Left Seat on GitHub and building the VPK there

You need only a web browser. GitHub builds the VPK for you; nothing has to be installed.

## 1. Make an account and an empty repository

1. Go to github.com and sign up (skip this if you already have an account). Confirm your email address.
2. Click the **+** at the top right, then **New repository**.
3. Repository name: `leftseat`. Choose **Public** (Actions builds are free and unlimited for public repositories).
4. Leave **Add a README**, **.gitignore** and **license** all **unticked**: the project already has them.
5. Click **Create repository**. You land on an empty page.

## 2. Upload the files (pick A or B)

### A. In the browser (easiest)

1. Unzip `leftseat.zip`. Open the `leftseat` folder.
2. On the empty repository page click **uploading an existing file**.
3. Select everything *inside* the `leftseat` folder (the folders `src`, `sce_sys`, `tools`, `.github`,
   `.devcontainer` and the loose files) and drag it onto the page. Do not drag the `leftseat` folder itself.
   - Mac: dot-folders such as `.github` are hidden in Finder. Press Cmd+Shift+. to show them first.
4. Wait for the file list to finish loading, type a message such as `First upload`, and click **Commit changes**.
5. Check the repository page. You should see `.github`, `.devcontainer`, `src`, `sce_sys`, `tools` and the loose files.
   If `.github` is missing: **Add file > Create new file**, type `.github/workflows/build.yml` as the name (the
   slashes create the folders), paste in the contents of that file from the zip, and commit.

### B. With git on your computer

```sh
cd leftseat
git init -b main
git add .
git commit -m "First upload"
git remote add origin https://github.com/YOUR-USERNAME/leftseat.git
git push -u origin main
```

GitHub does not accept your account password here. When asked to log in, use a personal access token
(github.com > Settings > Developer settings > Personal access tokens, with the `repo` and `workflow` permissions)
as the password, or install the GitHub CLI and run `gh auth login` first.

## 3. Let GitHub build the VPK

1. Open the **Actions** tab. If GitHub asks, click **I understand my workflows, go ahead and enable them**.
2. The upload started a run called **Build VPK**. Click it. It takes a few minutes: a yellow dot means running,
   a green tick means done, a red cross means it failed.
3. When it is green, scroll to **Artifacts** at the bottom of the run page and click **leftseat-vpk**. That
   downloads a zip with `leftseat.vpk` inside.
4. Unzip it, copy `leftseat.vpk` to your Vita and install it with VitaShell, as before.

If the run is red: click the run, click the **build** job, open the step with the red cross and copy the red
text at the bottom of it. Send that to me and I will fix the workflow.

To build again without changing anything: Actions tab > **Build VPK** > **Run workflow**.

## 4. Making a release people can download (optional)

1. On the repository page click **Releases** (right-hand side) > **Create a new release**.
2. Click **Choose a tag**, type `v1.0`, and choose **Create new tag**.
3. Give it a title such as `Left Seat 1.0` and a few lines of release notes, then click **Publish release**.
4. Publishing a new tag starts a build. After a few minutes `leftseat.vpk` appears under the release's
   **Assets**, and the Install section of the README points people there.

For later versions use `v1.1`, `v1.2` and so on. Also change `VITA_VERSION` in `CMakeLists.txt` if you want the
version shown on the Vita to match.

## 5. Changing things later

- **Edit one file:** open it on GitHub, click the pencil, change it, click **Commit changes**. A build starts
  automatically.
- **Replace several files:** **Add file > Upload files**, drop the new versions in (files with the same name
  and folder replace the old ones) and commit.
- **From a computer:** `git add .`, `git commit -m "what changed"`, `git push`.

Every push builds a fresh VPK on the Actions tab. Builds from earlier pushes stay there for 90 days.

## Good to know

- `build/`, `.vpk` and `.self` files are listed in `.gitignore`, so build output never gets uploaded.
- `fixpad.sh` can edit `src/pad.c` during a build. That only happens inside the GitHub build and is harmless.
- Your commits show your email address. To hide it: Settings > Emails > **Keep my email addresses private**.
- The licence in `LICENSE` is MIT with your name on it. Left Seat is homebrew and needs a modified Vita.
