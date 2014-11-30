package com.mx3;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;


public class GithubUsers extends Activity {
    static {
        System.loadLibrary("mx3_android");
    }

    private Api mApi;
    private UserListVmHandle mUserListHandle;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d("USERS", "Starting application");

        Http httpImpl = new AndroidHttp();
        EventLoop mainThread = new AndroidEventLoop();
        mApi = Api.createApi(this.getFilesDir().getAbsolutePath(), mainThread, httpImpl);
        mUserListHandle = mApi.observerUserList();
        mUserListHandle.start(new UserListVmObserver() {
            @Override
            public void onUpdate(UserListVm newData) {
                Log.d("USERS", "count: " + newData.count());
                if (newData.count() > 0) {
                    Log.d("USERS", newData.get(0).getName());
                }
            }
        });

        setContentView(R.layout.activity_github_users);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_github_users, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
