package com.mx3;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;


public class GithubUsers extends Activity {
    static {
        System.loadLibrary("mx3_android");
    }

    private Api mApi;
    private UserListVmHandle mUserListHandle;
    private GithubUserAdapter mAdapter;
    private ListView mListView;

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
            mAdapter = new GithubUserAdapter(GithubUsers.this, newData);
            Log.d("USERS", "count: " + mAdapter.getCount());
            mListView.setAdapter(mAdapter);
            mListView.deferNotifyDataSetChanged();
            }
        });

        setContentView(R.layout.activity_github_users);
        mListView = (ListView)findViewById(R.id.github_user_list);
    }

    @Override
    protected void onStop() {
        mUserListHandle.stop();
        super.onStop();
    }

    private class GithubUserAdapter extends BaseAdapter {
        private Context mContext;
        private LayoutInflater mInflater;
        private UserListVm mListVm;

        public GithubUserAdapter(Context context, UserListVm viewModel) {
            mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            mListVm = viewModel;
        }
        @Override
        public int getCount() {
            return mListVm.count();
        }

        @Override
        public long getItemId(int pos) {
            return pos;
        }

        @Override
        public Object getItem(int i) {
            return mListVm.get(i);
        }

        @Override
        public View getView(int i, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = mInflater.inflate(R.layout.github_user_cell, parent, false);
            }
            TextView textView = (TextView) convertView.findViewById(R.id.name_label);
            textView.setText(mListVm.get(i).getName());
            return convertView;
        }
    }
}
