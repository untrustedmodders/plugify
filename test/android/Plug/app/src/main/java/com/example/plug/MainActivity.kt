package com.example.plug

import android.os.Bundle
import android.os.Environment
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.plug.databinding.ActivityMainBinding
import java.io.File
import java.io.FileInputStream
import java.io.IOException


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        var dir = this.getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS)
        if (dir != null)
        {
            makePlugify(dir.absolutePath)
        }
    }

    /**
     * A native method that is implemented by the 'plug' native library,
     * which is packaged with this application.
     */
    external fun makePlugify(path: String)

    companion object {
        // Used to load the 'plug' library on application startup.
        init {
            System.loadLibrary("plug")
        }
    }
}