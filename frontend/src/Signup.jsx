import React from 'react'
import API from './api'

export default function Signup({onSignup}){
  const [name, setName] = React.useState('')
  const [email, setEmail] = React.useState('')
  const [password, setPassword] = React.useState('')
  const [err, setErr] = React.useState('')
  const [busy, setBusy] = React.useState(false)

  const submit = async (e) => {
    e.preventDefault()
    setBusy(true); setErr('')
    try {
      const data = await API.signup(name, email, password)
      onSignup(data)
    } catch (e) {
      setErr(e.message)
    } finally {
      setBusy(false)
    }
  }

  return (
    <form onSubmit={submit}>
      <div style={{display: 'grid', gap: 12}}>
        <label>Name<br/>
          <input value={name} onChange={e=>setName(e.target.value)} placeholder="Your name" required />
        </label>
        <label>Email<br/>
          <input type="email" value={email} onChange={e=>setEmail(e.target.value)} placeholder="you@example.com" required />
        </label>
        <label>Password<br/>
          <input type="password" value={password} onChange={e=>setPassword(e.target.value)} placeholder="Create a password" required />
        </label>
        <button disabled={busy}>{busy ? 'Creating…' : 'Create account'}</button>
        {err && <div style={{color:'crimson'}}>{err}</div>}
      </div>
    </form>
  )
}
