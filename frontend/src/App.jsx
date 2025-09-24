import React from 'react'
import Login from './Login'
import Signup from './Signup'
import API from './api'

function Navbar({user, onLoginClick, onSignupClick, onLogout}){
  return (
    <div style={{display:'flex', alignItems:'center', justifyContent:'space-between', padding:'12px 16px', borderBottom:'1px solid #ddd'}}>
      <div style={{fontSize:20, fontWeight:700}}>RateMyLandlord</div>
      <div>
        {!user ? (
          <>
            <button onClick={onLoginClick} style={{marginRight:8}}>Log in</button>
            <button onClick={onSignupClick}>Sign up</button>
          </>
        ) : (
          <div style={{display:'flex', alignItems:'center', gap:8}}>
            <span>Hi, {user.name}</span>
            <button onClick={onLogout}>Log out</button>
          </div>
        )}
      </div>
    </div>
  )
}

function SearchHome(){
  const [q, setQ] = React.useState('')
  const [list, setList] = React.useState([])
  const [err, setErr] = React.useState('')
  const [loading, setLoading] = React.useState(false)

  const doSearch = async (e)=>{
    e && e.preventDefault()
    setErr(''); setLoading(true)
    try {
      const results = await API.searchLandlords(q.trim())
      setList(results)
    } catch(e){
      setErr(e.message)
    } finally {
      setLoading(false)
    }
  }

  React.useEffect(()=>{ doSearch() }, [])

  return (
    <div style={{maxWidth:800, margin:'24px auto', padding:'0 16px', fontFamily:'system-ui, sans-serif'}}>
      <form onSubmit={doSearch} style={{display:'flex', gap:8}}>
        <input
          value={q}
          onChange={e=>setQ(e.target.value)}
          placeholder="Search landlords by name…"
          style={{flex:1, padding:'10px 12px'}}
        />
        <button>Search</button>
      </form>
      {err && <div style={{color:'crimson', marginTop:12}}>{err}</div>}
      <div style={{marginTop:16}}>
        {loading ? 'Searching…' : (
          list.length ? (
            <ul>
              {list.map((ll)=> (
                <li key={ll.landlord_id} style={{marginBottom:12}}>
                  <div style={{fontWeight:600}}>{ll.name}</div>
                  <div style={{fontSize:12, opacity:.8}}>{ll.contact?.email} · {ll.contact?.phone}</div>
                  <div style={{fontSize:12, opacity:.8}}>{(ll.properties||[]).length} properties</div>
                </li>
              ))}
            </ul>
          ) : <div style={{opacity:.7}}>No landlords found.</div>
        )}
      </div>
    </div>
  )
}

export default function App(){
  const [token, setToken] = React.useState(localStorage.getItem('token'))
  const [user, setUser] = React.useState(null)
  const [showLogin, setShowLogin] = React.useState(false)
  const [showSignup, setShowSignup] = React.useState(false)

  React.useEffect(()=>{
    if(!token){ setUser(null); return }
    API.me(token).then(setUser).catch(()=> setUser(null))
  }, [token])

  return (
    <div>
      <Navbar
        user={user}
        onLoginClick={()=>{ setShowLogin(true); setShowSignup(false) }}
        onSignupClick={()=>{ setShowSignup(true); setShowLogin(false) }}
        onLogout={()=>{ localStorage.removeItem('token'); setToken(null) }}
      />
      <SearchHome />

      {showLogin && (
        <div style={{position:'fixed', inset:0, background:'rgba(0,0,0,.25)'}} onClick={()=>setShowLogin(false)}>
          <div style={{background:'#fff', padding:16, maxWidth:360, margin:'10% auto'}} onClick={e=>e.stopPropagation()}>
            <Login onLogin={({token})=>{ localStorage.setItem('token', token); setToken(token); setShowLogin(false) }} />
          </div>
        </div>
      )}

      {showSignup && (
        <div style={{position:'fixed', inset:0, background:'rgba(0,0,0,.25)'}} onClick={()=>setShowSignup(false)}>
          <div style={{background:'#fff', padding:16, maxWidth:360, margin:'10% auto'}} onClick={e=>e.stopPropagation()}>
            <Signup onSignup={({token})=>{ localStorage.setItem('token', token); setToken(token); setShowSignup(false) }} />
          </div>
        </div>
      )}
    </div>
  )
}
