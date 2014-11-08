// $Id$
//
// Container with name, TClonesArray and cuts for jets
//
// Author: M. Verweij

#include <TClonesArray.h>

#include "AliEmcalJet.h"
#include "AliVEvent.h"
#include "AliLog.h"
#include "AliEMCALGeometry.h"
#include "AliParticleContainer.h"
#include "AliClusterContainer.h"
#include "AliLocalRhoParameter.h"
#include "AliStackPartonInfo.h"

#include "AliJetContainer.h"

ClassImp(AliJetContainer)

//________________________________________________________________________
AliJetContainer::AliJetContainer():
  AliEmcalContainer("AliJetContainer"),
  fJetAcceptanceType(kUser),
  fJetRadius(0),
  fRhoName(),
  fLocalRhoName(),
  fRhoMassName(),
  fPartonInfoName(),
  fFlavourSelection(0),
  fPtBiasJetTrack(0),
  fPtBiasJetClus(0),
  fJetPtCut(1),
  fJetAreaCut(-1),
  fAreaEmcCut(0),
  fJetMinEta(-0.9),
  fJetMaxEta(0.9),
  fJetMinPhi(-10),
  fJetMaxPhi(10),
  fMaxClusterPt(1000),
  fMaxTrackPt(100),
  fZLeadingEmcCut(10.),
  fZLeadingChCut(10.),
  fNEFMinCut(-10.),
  fNEFMaxCut(10.),
  fLeadingHadronType(0),
  fNLeadingJets(1),
  fJetBitMap(0),
  fJetTrigger(0),
  fTagStatus(-1),
  fParticleContainer(0),
  fClusterContainer(0),
  fRho(0),
  fLocalRho(0),
  fRhoMass(0),
  fPartonsInfo(0),
  fGeom(0),
  fRunNumber(0)
{
  // Default constructor.

  fClassName = "AliEmcalJet";
}

//________________________________________________________________________
AliJetContainer::AliJetContainer(const char *name):
  AliEmcalContainer(name),
  fJetAcceptanceType(kUser),
  fJetRadius(0),
  fRhoName(),
  fLocalRhoName(),
  fRhoMassName(),
  fPartonInfoName(),
  fFlavourSelection(0),
  fPtBiasJetTrack(0),
  fPtBiasJetClus(0),
  fJetPtCut(1),
  fJetAreaCut(-1),
  fAreaEmcCut(0),
  fJetMinEta(-0.9),
  fJetMaxEta(0.9),
  fJetMinPhi(-10),
  fJetMaxPhi(10),
  fMaxClusterPt(1000),
  fMaxTrackPt(100),
  fZLeadingEmcCut(10.),
  fZLeadingChCut(10.),
  fNEFMinCut(-10.),
  fNEFMaxCut(10.),
  fLeadingHadronType(0),
  fNLeadingJets(1),
  fJetBitMap(0),
  fJetTrigger(0),
  fTagStatus(-1),
  fParticleContainer(0),
  fClusterContainer(0),
  fRho(0),
  fLocalRho(0),
  fRhoMass(0),
  fPartonsInfo(0),
  fGeom(0),
  fRunNumber(0)
{
  // Standard constructor.

  fClassName = "AliEmcalJet";
}

//________________________________________________________________________
void AliJetContainer::SetArray(AliVEvent *event) 
{
  // Set jet array

  AliEmcalContainer::SetArray(event);

  if(fJetAcceptanceType==kTPC) {
    AliDebug(2,Form("%s: set TPC acceptance cuts",GetName()));
    SetJetEtaPhiTPC();
  }
  else if(fJetAcceptanceType==kEMCAL) {
    AliDebug(2,Form("%s: set EMCAL acceptance cuts",GetName()));
    SetJetEtaPhiEMCAL();
 }
}


//________________________________________________________________________
void AliJetContainer::SetEMCALGeometry() {
  fGeom = AliEMCALGeometry::GetInstance();
  if (!fGeom) {
    AliError(Form("%s: Can not create geometry", GetName()));
    return;
  }
}

//________________________________________________________________________
void AliJetContainer::LoadRho(AliVEvent *event)
{
  // Load rho

  if (!fRhoName.IsNull() && !fRho) {
    fRho = dynamic_cast<AliRhoParameter*>(event->FindListObject(fRhoName));
    if (!fRho) {
      AliError(Form("%s: Could not retrieve rho %s!", GetName(), fRhoName.Data()));
      return;
    }
  }
}

//________________________________________________________________________
void AliJetContainer::LoadLocalRho(AliVEvent *event)
{
  // Load local rho

  if (!fLocalRhoName.IsNull() && !fLocalRho) {
    fLocalRho = dynamic_cast<AliLocalRhoParameter*>(event->FindListObject(fLocalRhoName));
    if (!fLocalRho) {
      AliError(Form("%s: Could not retrieve rho %s!", GetName(), fLocalRhoName.Data()));
      return;
    }
  }
}

//________________________________________________________________________
void AliJetContainer::LoadRhoMass(AliVEvent *event)
{
  // Load rho

  if (!fRhoMassName.IsNull() && !fRhoMass) {
    fRhoMass = dynamic_cast<AliRhoParameter*>(event->FindListObject(fRhoMassName));
    if (!fRhoMass) {
      AliError(Form("%s: Could not retrieve rho_mass %s!", GetName(), fRhoMassName.Data()));
      return;
    }
  }
}
//________________________________________________________________________
void AliJetContainer::LoadPartonsInfo(AliVEvent *event)
{
    // Load parton info
    
    if (!fPartonInfoName.IsNull() && !fPartonsInfo) {
        fPartonsInfo = dynamic_cast<AliStackPartonInfo*>(event->FindListObject(fPartonInfoName));
        if (!fPartonsInfo) {
           AliError(Form("%s: Could not retrieve parton infos! %s!", GetName(), fPartonInfoName.Data()));            return;
        }
    }
}



//________________________________________________________________________
AliEmcalJet* AliJetContainer::GetLeadingJet(const char* opt)
{
  // Get the leading jet; if opt contains "rho" the sorting is according to pt-A*rho

  TString option(opt);
  option.ToLower();

  Int_t tempID = fCurrentID;

  AliEmcalJet *jetMax = GetNextAcceptJet(0);
  AliEmcalJet *jet = 0;

  if (option.Contains("rho")) {
    while ((jet = GetNextAcceptJet())) {
      if ( (jet->Pt()-jet->Area()*GetRhoVal()) > (jetMax->Pt()-jetMax->Area()*GetRhoVal()) )
	jetMax = jet;
    }
  }
  else {
    while ((jet = GetNextAcceptJet())) {
      if (jet->Pt() > jetMax->Pt()) jetMax = jet;
    }
  }

  fCurrentID = tempID;

  return jetMax;
}

//________________________________________________________________________
AliEmcalJet* AliJetContainer::GetJet(Int_t i) const {

  //Get i^th jet in array

  if(i<0 || i>fClArray->GetEntriesFast()) return 0;
  AliEmcalJet *jet = static_cast<AliEmcalJet*>(fClArray->At(i));
  return jet;

}

//________________________________________________________________________
AliEmcalJet* AliJetContainer::GetAcceptJet(Int_t i) {

  //Only return jet if is accepted

  AliEmcalJet *jet = GetJet(i);
  if(!AcceptJet(jet)) return 0;

  return jet;
}

//________________________________________________________________________
AliEmcalJet* AliJetContainer::GetJetWithLabel(Int_t lab) const {

  //Get particle with label lab in array
  
  Int_t i = GetIndexFromLabel(lab);
  return GetJet(i);
}

//________________________________________________________________________
AliEmcalJet* AliJetContainer::GetAcceptJetWithLabel(Int_t lab) {

  //Get particle with label lab in array
  
  Int_t i = GetIndexFromLabel(lab);
  return GetAcceptJet(i);
}

//________________________________________________________________________
AliEmcalJet* AliJetContainer::GetNextAcceptJet(Int_t i) {

  //Get next accepted jet; if i >= 0 (re)start counter from i; return 0 if no accepted jet could be found

  if (i>=0) fCurrentID = i;

  const Int_t njets = GetNEntries();
  AliEmcalJet *jet = 0;
  while (fCurrentID < njets && !jet) { 
    jet = GetAcceptJet(fCurrentID);
    fCurrentID++;
  }

  return jet;
}

//________________________________________________________________________
AliEmcalJet* AliJetContainer::GetNextJet(Int_t i) {

  //Get next jet; if i >= 0 (re)start counter from i; return 0 if no jet could be found

  if (i>=0) fCurrentID = i;

  const Int_t njets = GetNEntries();
  AliEmcalJet *jet = 0;
  while (fCurrentID < njets && !jet) { 
    jet = GetJet(fCurrentID);
    fCurrentID++;
  }

  return jet;
}

//________________________________________________________________________
Double_t AliJetContainer::GetJetPtCorr(Int_t i) const {
  AliEmcalJet *jet = GetJet(i);

  return jet->Pt() - fRho->GetVal()*jet->Area();
}

//________________________________________________________________________
Double_t AliJetContainer::GetJetPtCorrLocal(Int_t i) const {
  AliEmcalJet *jet = GetJet(i);

  return jet->Pt() - fLocalRho->GetLocalVal(jet->Phi(), fJetRadius)*jet->Area();
}

//________________________________________________________________________
void AliJetContainer::GetMomentum(TLorentzVector &mom, Int_t i) const
{
  //Get momentum of the i^th jet in array

  AliEmcalJet *jet = GetJet(i);
  if(jet) jet->GetMom(mom);
}

//________________________________________________________________________
Bool_t AliJetContainer::AcceptBiasJet(const AliEmcalJet *jet)
{ 
  // Accept jet with a bias.

  if (fLeadingHadronType == 0) {
    if (jet->MaxTrackPt() < fPtBiasJetTrack) return kFALSE;
  }
  else if (fLeadingHadronType == 1) {
    if (jet->MaxClusterPt() < fPtBiasJetClus) return kFALSE;
  }
  else {
    if (jet->MaxTrackPt() < fPtBiasJetTrack && jet->MaxClusterPt() < fPtBiasJetClus) return kFALSE;
  }

  return kTRUE;
}

//________________________________________________________________________
Bool_t AliJetContainer::AcceptJet(const AliEmcalJet *jet)
{   
   // Return true if jet is accepted.

  fRejectionReason = 0;

  if (!jet) {
    AliDebug(11,"No jet found");
    fRejectionReason |= kNullObject;
    return kFALSE;
  }

  if (jet->Pt() <= fJetPtCut) {
    AliDebug(11,Form("Cut rejecting jet: JetPtCut %.1f",fJetPtCut));
    fRejectionReason |= kPtCut;
    return kFALSE;
  }

  Double_t jetPhi = jet->Phi();
  Double_t jetEta = jet->Eta();
   
  // if limits are given in (-pi, pi) range
  if (fJetMinPhi < 0) jetPhi -= TMath::Pi() * 2;
   
  if (jetEta < fJetMinEta || jetEta > fJetMaxEta || jetPhi < fJetMinPhi || jetPhi > fJetMaxPhi) {
    AliDebug(11,"Cut rejecting jet: Acceptance");
    fRejectionReason |= kAcceptanceCut;
    return kFALSE;
  }

  if (jet->TestBits(fJetBitMap) != (Int_t)fJetBitMap) {
    AliDebug(11,"Cut rejecting jet: Bit map");
    fRejectionReason |= kBitMapCut;
    return kFALSE;
  }

  if (jet->Area() <= fJetAreaCut)  {
    AliDebug(11,"Cut rejecting jet: Area");
    fRejectionReason |= kAreaCut;
    return kFALSE;
  }

  if (jet->AreaEmc() < fAreaEmcCut) {
    AliDebug(11,"Cut rejecting jet: AreaEmc");
    fRejectionReason |= kAreaEmcCut;
    return kFALSE;
  }
   
  if (fZLeadingChCut < 1 && GetZLeadingCharged(jet) > fZLeadingChCut) {
    AliDebug(11,"Cut rejecting jet: ZLeading");
    fRejectionReason |= kZLeadingChCut;
    return kFALSE;
  }
   
  if (fZLeadingEmcCut < 1 && GetZLeadingEmc(jet) > fZLeadingEmcCut) {
    AliDebug(11,"Cut rejecting jet: ZLeadEmc");
    fRejectionReason |= kZLeadingEmcCut;
    return kFALSE;
  }

  if (jet->NEF() < fNEFMinCut || jet->NEF() > fNEFMaxCut) {
    AliDebug(11,"Cut rejecting jet: NEF");
    fRejectionReason |= kNEFCut;
    return kFALSE;
  }
   
  if (!AcceptBiasJet(jet)) {
    AliDebug(11,"Cut rejecting jet: Bias");
    fRejectionReason |= kMinLeadPtCut;
    return kFALSE;
  }

  if (jet->MaxTrackPt() > fMaxTrackPt) {
    AliDebug(11,"Cut rejecting jet: MaxTrackPt");
    fRejectionReason |= kMaxTrackPtCut;
    return kFALSE;

  }

  if (jet->MaxClusterPt() > fMaxClusterPt) {
    AliDebug(11,"Cut rejecting jet: MaxClusPt");
    fRejectionReason |= kMaxClusterPtCut;
    return kFALSE;
  }
   
  if (fFlavourSelection != 0 && !jet->TestFlavourTag(fFlavourSelection)) {
    AliDebug(11,"Cut rejecting jet: Flavour");
    fRejectionReason |= kFlavourCut;
    return kFALSE;
  }
   
  if(fTagStatus>-1 && jet->GetTagStatus()!=fTagStatus) {
    AliDebug(11,"Cut rejecting jet: tag status");
    fRejectionReason |= kTagStatus;
    return kFALSE;
  }

  return kTRUE;
}

//________________________________________________________________________
Double_t AliJetContainer::GetLeadingHadronPt(const AliEmcalJet *jet) const
{
  if (fLeadingHadronType == 0)       // charged leading hadron
    return jet->MaxTrackPt();
  else if (fLeadingHadronType == 1)  // neutral leading hadron
    return jet->MaxClusterPt();
  else                               // charged or neutral
    return jet->MaxPartPt();
}

//________________________________________________________________________
void AliJetContainer::GetLeadingHadronMomentum(TLorentzVector &mom, const AliEmcalJet *jet) const
{
  Double_t maxClusterPt = 0;
  Double_t maxClusterEta = 0;
  Double_t maxClusterPhi = 0;
  
  Double_t maxTrackPt = 0;
  Double_t maxTrackEta = 0;
  Double_t maxTrackPhi = 0;
      
  if (fClusterContainer && fClusterContainer->GetArray() && (fLeadingHadronType == 1 || fLeadingHadronType == 2)) {
    AliVCluster *cluster = jet->GetLeadingCluster(fClusterContainer->GetArray());
    if (cluster) {
      TLorentzVector nPart;
      cluster->GetMomentum(nPart, const_cast<Double_t*>(fVertex));
      
      maxClusterEta = nPart.Eta();
      maxClusterPhi = nPart.Phi();
      maxClusterPt = nPart.Pt();
    }
  }
      
  if (fParticleContainer && fParticleContainer->GetArray() && (fLeadingHadronType == 0 || fLeadingHadronType == 2)) {
    AliVParticle *track = jet->GetLeadingTrack(fParticleContainer->GetArray());
    if (track) {
      maxTrackEta = track->Eta();
      maxTrackPhi = track->Phi();
      maxTrackPt = track->Pt();
    }
  }
      
  if (maxTrackPt > maxClusterPt) 
    mom.SetPtEtaPhiM(maxTrackPt,maxTrackEta,maxTrackPhi,0.139);
  else 
    mom.SetPtEtaPhiM(maxClusterPt,maxClusterEta,maxClusterPhi,0.139);
}

//________________________________________________________________________
Double_t AliJetContainer::GetZLeadingEmc(const AliEmcalJet *jet) const
{

  if (fClusterContainer && fClusterContainer->GetArray()) {
    TLorentzVector mom;
    
    AliVCluster *cluster = jet->GetLeadingCluster(fClusterContainer->GetArray());
    if (cluster) {
      cluster->GetMomentum(mom, const_cast<Double_t*>(fVertex));
      
      return GetZ(jet,mom);
    }
    else
      return -1;
  }
  else
    return -1;
}

//________________________________________________________________________
Double_t AliJetContainer::GetZLeadingCharged(const AliEmcalJet *jet) const
{

  if (fParticleContainer && fParticleContainer->GetArray() ) {
    TLorentzVector mom;
    
    AliVParticle *track = jet->GetLeadingTrack(fParticleContainer->GetArray());
    if (track) {
      mom.SetPtEtaPhiM(track->Pt(),track->Eta(),track->Phi(),0.139);
      
      return GetZ(jet,mom);
    }
    else
      return -1;
  }
  else
    return -1;
}

//________________________________________________________________________
Double_t AliJetContainer::GetZ(const AliEmcalJet *jet, TLorentzVector mom) const
{

  Double_t pJetSq = jet->Px()*jet->Px() + jet->Py()*jet->Py() + jet->Pz()*jet->Pz();

  if(pJetSq>1e-6)
    return (mom.Px()*jet->Px() + mom.Py()*jet->Py() + mom.Pz()*jet->Pz())/pJetSq;
  else {
    AliWarning(Form("%s: strange, pjet*pjet seems to be zero pJetSq: %f",GetName(), pJetSq));
    return -1;
  }

}

//________________________________________________________________________
void AliJetContainer::SetJetEtaPhiEMCAL()
{
  //Set default cuts for full jets

  if(!fGeom) SetEMCALGeometry();
  if(fGeom) {
    SetJetEtaLimits(fGeom->GetArm1EtaMin() + fJetRadius, fGeom->GetArm1EtaMax() - fJetRadius);

    if(fRunNumber>=177295 && fRunNumber<=197470) //small SM masked in 2012 and 2013
      SetJetPhiLimits(1.405+fJetRadius,3.135-fJetRadius);
    else
      SetJetPhiLimits(fGeom->GetArm1PhiMin() * TMath::DegToRad() + fJetRadius, fGeom->GetArm1PhiMax() * TMath::DegToRad() - fJetRadius);
  }
  else {
    AliWarning("Could not get instance of AliEMCALGeometry. Using manual settings for EMCAL year 2011!!");
    SetJetEtaLimits(-0.7+fJetRadius,0.7-fJetRadius);
    SetJetPhiLimits(1.405+fJetRadius,3.135-fJetRadius);
  }
}

//________________________________________________________________________
void AliJetContainer::SetJetEtaPhiTPC()
{
  //Set default cuts for charged jets

  SetJetEtaLimits(-0.9+fJetRadius, 0.9-fJetRadius);
  SetJetPhiLimits(-10, 10);
}

//________________________________________________________________________
void AliJetContainer::PrintCuts() 
{
  // Reset cuts to default values

  Printf("PtBiasJetTrack: %f",fPtBiasJetTrack);
  Printf("PtBiasJetClus: %f",fPtBiasJetClus);
  Printf("JetPtCut: %f", fJetPtCut);
  Printf("JetAreaCut: %f",fJetAreaCut);
  Printf("AreaEmcCut: %f",fAreaEmcCut);
  Printf("JetMinEta: %f", fJetMinEta);
  Printf("JetMaxEta: %f", fJetMaxEta);
  Printf("JetMinPhi: %f", fJetMinPhi);
  Printf("JetMaxPhi: %f", fJetMaxPhi);
  Printf("MaxClusterPt: %f",fMaxClusterPt);
  Printf("MaxTrackPt: %f",fMaxTrackPt);
  Printf("LeadingHadronType: %d",fLeadingHadronType);
  Printf("ZLeadingEmcCut: %f",fZLeadingEmcCut);
  Printf("ZLeadingChCut: %f",fZLeadingChCut);

}

//________________________________________________________________________
void AliJetContainer::ResetCuts() 
{
  // Reset cuts to default values

  fPtBiasJetTrack = 0;
  fPtBiasJetClus = 0;
  fJetPtCut = 1;
  fJetAreaCut = -1;
  fAreaEmcCut = 0;
  fJetMinEta = -0.9;
  fJetMaxEta = 0.9;
  fJetMinPhi = -10;
  fJetMaxPhi = 10;
  fMaxClusterPt = 1000;
  fMaxTrackPt = 100;
  fLeadingHadronType = 0;
  fZLeadingEmcCut = 10.;
  fZLeadingChCut = 10.;
}

//________________________________________________________________________
void AliJetContainer::SetClassName(const char *clname)
{
  // Set the class name

  TClass cls(clname);
  if (cls.InheritsFrom("AliEmcalJet")) fClassName = clname;
  else AliError(Form("Unable to set class name %s for a AliJetContainer, it must inherits from AliEmcalJet!",clname));
}

//________________________________________________________________________
Double_t AliJetContainer::GetFractionSharedPt(AliEmcalJet *jet1) const
{
  //
  // Get fraction of shared pT between matched full and charged jet
  // Uses charged jet pT as baseline: fraction = \Sum_{const,full jet} pT,const,i / pT,jet,ch
  // Only works if tracks array of both jets is the same
  //

  AliEmcalJet *jet2 = jet1->ClosestJet();
  if(!jet2) return -1;

  Double_t fraction = 0.;
  Double_t jetPt2 = jet2->Pt();
 
  if(jetPt2>0) {
    Double_t sumPt = 0.;
    AliVParticle *vpf = 0x0;
    Int_t iFound = 0;
    for(Int_t icc=0; icc<jet2->GetNumberOfTracks(); icc++) {
      Int_t idx = (Int_t)jet2->TrackAt(icc);
      iFound = 0;
      for(Int_t icf=0; icf<jet1->GetNumberOfTracks(); icf++) {
	if(idx == jet1->TrackAt(icf) && iFound==0 ) {
	  iFound=1;
	  vpf = static_cast<AliVParticle*>(jet1->TrackAt(icf, fParticleContainer->GetArray()));
	  if(vpf) sumPt += vpf->Pt();
	  continue;
	}
      }
    }
    fraction = sumPt/jetPt2;
  } else 
    fraction = -1;
  
  return fraction;
}

